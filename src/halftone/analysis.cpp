
/*****
*
*   LEGAL NOTICE:
*
*      This code is free for non-commercial use only.
*      For any other use, please contact the authors.
*
*
*
*
*   Authors: Jianghao Chang, Benoit Alain
*
*   Contact (paper): jianghao.chang@umontreal.ca
*   Contact (code): benoit.alain@gmail.com
*
*****/


#include <opencv2/core/core_c.h>
#include "halftone/util.h"
#include "halftone/analysis.h"

/** \file	analysis.c
 *
 *  An analysis tool that works on a local window of prescribed size is given here.
 *
 *  Implementation uses a discrete short-time fourier transform to obtain a local frequency spectrum.
 *  That spectrum is then weighted to favor the frequencies that our system needs to handle most carefully.
 *  Local orientation and frequency are obtained from the location of the maximal weighted oriented frequency.
 *  Local contrast is obtained from taking the mean of both local extrema within period range.
 *
 *  This heuristic analysis tool may differ from that described in the paper.
 *  We found it to be faster and still give very satisfying results.
 *  IMPORTANT: any different analysis tool can be used as long as calibration is done in accordance.
 */

/** \typedef	AnalysisKernel */

/// Fits local analysis window. Has constant dimensions to allow loop unrolling.
/// The analysis kernel data is accessible by index or as a bloc.
/// Uses an independant internal color convention, that is, a single floating-point value ranging from 0 (black) to 1 (white).
/// This is required for the preference function to be accurately balanced.
/// Analysed local 'contrast' value is scaled to fit the MAX_CONTRAST external constant.
/// IMPORTANT: data is stored without padding, in column-major order. 'offset = x*ANALYSIS_KERNEL_LENGTH + y'
typedef union
{
	float v[ANALYSIS_KERNEL_LENGTH*ANALYSIS_KERNEL_LENGTH];
	float m[ANALYSIS_KERNEL_LENGTH][ANALYSIS_KERNEL_LENGTH];
} AnalysisKernel;

/// Design parameter that determines the relative importance of the central region of the analysis window.
/// Even if a larger kernel size is used, this parameter may remain unchanged.
static const float SPATIAL_WINDOWING_SIGMA = 3.0f;
static const float GABOR_QUADRATURE_SIGMA = 1.0f;

/// Handy pre-evalulations of 'spatialWindowingFunction()' and 'spectralPreferenceFunction()'.
AnalysisKernel spatialWindowingLookup, spectralPreferenceLookup;

/// The windowing function puts emphasis on the central pixel of the window.
/// It preserves local hierarchy of frequencies and orientations but affects overall contrast.
static double spatialWindowingFunction(int x, int y)
{
	return exp(-(x*x+y*y)/(2*SPATIAL_WINDOWING_SIGMA*SPATIAL_WINDOWING_SIGMA));
}
/// The preference function is needed to resolve simultaneous local frequencies.
/// It favors the frequencies for which our algorithm performs best.
static double spectralPreferenceFunction(int x, int y)
{
	double r = sqrt(x*x + y*y) / ANALYSIS_KERNEL_LENGTH;
	double sigma = r < 0.28 ? 0.12 : 0.33;

	return (r+0.03) * exp(-(r-0.28)*(r-0.28) / (2*sigma*sigma));
}

/// A routine that serves as an interface between the two data formats, and hides boundary checking details
/// Implementation uses floating-point values in [0, 1] for analysis kernel
static void extractKernel(Image *img, int x, int y, AnalysisKernel *out)
{
	int xStart = x - ANALYSIS_KERNEL_HALFLENGTH;
	int yStart = y - ANALYSIS_KERNEL_HALFLENGTH;
	int i, j;

	for(i = 0;i < ANALYSIS_KERNEL_LENGTH;i++)
		for(j = 0;j < ANALYSIS_KERNEL_LENGTH;j++)
			out->m[i][j] = (pixelPointer(img, xStart + i, yStart + j)->gray - MIN_TONE) / (MAX_TONE - MIN_TONE);
}
/// A very straightforward routine that will hopefully be inlined and fully unrolled by the compiler
static void multiplyKernel(AnalysisKernel *ker1, AnalysisKernel *ker2, AnalysisKernel *out)
{
	int i;
	for(i = 0;i < (ANALYSIS_KERNEL_LENGTH*ANALYSIS_KERNEL_LENGTH);i++)
		out->v[i] = ker1->v[i] * ker2->v[i];
}

/// Used by 'logPowerSpectrum()'. We used OpenCV's library to compute the DFT, but using FFTW would be better.
static void cvPowerSpectrum(CvMat *space, CvMat *spectre)
{
	CvMat *complexMat = cvCreateMat(space->rows, space->cols, CV_32FC2);
	cvZero(spectre);
	cvMerge(space, spectre, 0, 0, complexMat);
	cvDFT(complexMat, complexMat, CV_DXT_FORWARD, 0);
	cvSplit(complexMat, space, spectre, 0, 0);
	cvCartToPolar(space, spectre, spectre, 0, 0);
	cvReleaseMat(&complexMat);
}
/// The "power spectrum" is just a discrete fourier transform without phase information.
/// It is scaled logarithmically to allow the preference function to operate convincingly.
/// Note that junk may be left in the superfluous upper half of the spectrum.
static void logPowerSpectrum(AnalysisKernel *spatialData, AnalysisKernel *spectrum, AnalysisKernel *logSpectrum)
{
	CvMat spatialMat, spectralMat;
	int i;

	spatialMat = cvMat(ANALYSIS_KERNEL_LENGTH, ANALYSIS_KERNEL_LENGTH, CV_32FC1, spatialData->v);
	spectralMat = cvMat(ANALYSIS_KERNEL_LENGTH, ANALYSIS_KERNEL_LENGTH, CV_32FC1, spectrum->v);
	cvPowerSpectrum(&spatialMat, &spectralMat);

	for(i = 0;i < (ANALYSIS_KERNEL_LENGTH*ANALYSIS_KERNEL_LENGTH);i++)
		logSpectrum->v[i] = (float) log(1 + spectrum->v[i]);
	return;
}
/// Locates maximal value within the active half of given spectrum
static void locateSpectrumMaximum(AnalysisKernel *kernel, int *xMax, int *yMax)
{
	int i, j;

	*xMax = *yMax = 0;

	// Avoid DC and redundant frequencies on the upper half of the spectrum
	for(i = 0, j = 1;j <= ANALYSIS_KERNEL_HALFLENGTH;j++)
		if(kernel->m[i][j] > kernel->m[*xMax][*yMax])
			*xMax = i, *yMax = j;
	for(i = 1;i < ANALYSIS_KERNEL_LENGTH;i++) for(j = 0;j <= ANALYSIS_KERNEL_HALFLENGTH;j++)
		if(kernel->m[i][j] > kernel->m[*xMax][*yMax])
			*xMax = i, *yMax = j;
}
/// The local extrema approach gives a good qualitative idea of what the local range of gray is, which corresponds to local contrast.
/// Implementation does not force the extrema to be in accordance with wave orientation, because orientation is generally poor with a small kernel size.
/// In principle, the extrema should align naturally with the orientation, that is, in the direction of maximal gradient.
static void findCloseExtrema(AnalysisKernel *window, int xWaveNumber, int yWaveNumber, float *min, float *max)
{
	// Express the x and y wave numbers with their equivalent of shortest module
	int xSignedWaveNumber = xWaveNumber < ANALYSIS_KERNEL_HALFLENGTH ? xWaveNumber : xWaveNumber - ANALYSIS_KERNEL_LENGTH;
	int ySignedWaveNumber = yWaveNumber;

	// Compute the frequency and the corresponding period
	double frequency = sqrt(xSignedWaveNumber*xSignedWaveNumber + ySignedWaveNumber*ySignedWaveNumber) * (2*PI) / ANALYSIS_KERNEL_LENGTH;
	double period = (2*PI) / (frequency + 1);
	double halfPeriod = period / 2;

	// Allow the extrema to be found within the period, centered on pixel of interest, but not specifically at given orientation
	int halfRange = (int) ceil(halfPeriod);
	int start = halfRange < ANALYSIS_KERNEL_HALFLENGTH ? ANALYSIS_KERNEL_HALFLENGTH - halfRange : 0;
	int end = halfRange+1 < ANALYSIS_KERNEL_HALFLENGTH ? ANALYSIS_KERNEL_HALFLENGTH + halfRange+1 : ANALYSIS_KERNEL_LENGTH;

	int i, j, iMin, jMin, iMax, jMax, x, y;
	iMin = iMax = jMin = jMax = ANALYSIS_KERNEL_HALFLENGTH;

	// Loop over at least all candidate pixels
	for(i = start;i < end;i++) for(j = start;j < end;j++)
	{
		int x = i - ANALYSIS_KERNEL_HALFLENGTH, y = j - ANALYSIS_KERNEL_HALFLENGTH;

		// Don't overextend
		if(x*x + y*y <= halfPeriod*halfPeriod)
		{
			if(window->m[i][j] < window->m[iMin][jMin])
				iMin = i, jMin = j;
			if(window->m[i][j] > window->m[iMax][jMax])
				iMax = i, jMax = j;
		}
	}

	*min = window->m[iMin][jMin];
	*max = window->m[iMax][jMax];
}
//TODO: 
/// The high-level general-purpose frequency analysis tool.
/// Computes a single pixel's structural profile, that is, the input image's pixel of focus.
/// IMPORTANT: has undefined behaviour if called before 'readyFrequencyAnalysis()'.
void localFrequencyAnalysis(Image *img, int x, int y, FrequencyContent *freqContent)
{
	LocalFrequencySignature *out = frequencyPointer(freqContent, x, y);
	AnalysisKernel imageWindow, focusWindow, powerSpectrum, logSpectrum;
	float min, max, gradient;
	int xWaveNumber, yWaveNumber;

	// Focus on a region around central pixel
	extractKernel(img, x, y, &imageWindow);
	multiplyKernel(&imageWindow, &spatialWindowingLookup, &focusWindow);

	// Compute the power spectrum and a preference-weighted version of it
	logPowerSpectrum(&focusWindow, &powerSpectrum, &logSpectrum);
	multiplyKernel(&logSpectrum, &spectralPreferenceLookup, &logSpectrum);

	// Find the most active region of the weighted power spectrum
	locateSpectrumMaximum(&logSpectrum, &xWaveNumber, &yWaveNumber);

	// Compute local contrast from half the local range of gray intensities
	findCloseExtrema(&imageWindow, xWaveNumber, yWaveNumber, &min, &max);

	out->xWaveNumber = xWaveNumber;
	out->yWaveNumber = yWaveNumber;
	out->contrast = (max - min) * MAX_CONTRAST;
}

/// An initialisation routine for reusable data. Must be called once.
/// IMPORTANT: will not be called by 'localFrequencyAnalysis()' to avoid conflicts with multiple threads.
void readyFrequencyAnalysis()
{
	int i, j, x, y;
	for(i = 0;i < ANALYSIS_KERNEL_LENGTH;i++) for(j = 0;j < ANALYSIS_KERNEL_LENGTH;j++)
	{
		x = i < ANALYSIS_KERNEL_HALFLENGTH ? i : i - ANALYSIS_KERNEL_LENGTH;
		y = j < ANALYSIS_KERNEL_HALFLENGTH ? j : j - ANALYSIS_KERNEL_LENGTH;

		spatialWindowingLookup.m[i][j] = (float) spatialWindowingFunction(i - ANALYSIS_KERNEL_HALFLENGTH, j - ANALYSIS_KERNEL_HALFLENGTH);
		spectralPreferenceLookup.m[i][j] = (float) spectralPreferenceFunction(x, y);
	}
}
