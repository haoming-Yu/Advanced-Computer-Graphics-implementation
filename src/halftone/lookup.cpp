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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "halftone/util.h"
#include "halftone/analysis.h"
#include "halftone/lookup.h"

// Precomputed data tables are found in these pseudo header file.
#include STANDARD_CALIBRATION_HEADER
#include ADAPTIVE_CALIBRATION_HEADER

#define MIN_VARIANCE		(MIN_PARAM_SIGMA * MIN_PARAM_SIGMA)
#define MAX_VARIANCE		(MAX_PARAM_SIGMA * MAX_PARAM_SIGMA)
#define MIN_ANISOTROPY		MIN_PARAM_A
#define MAX_ANISOTROPY		MAX_PARAM_A

/// Lookup tables. External access to their data is done through the 'get' functions.
ThresholdFilter gaborThresholdFilter[ANALYSIS_KERNEL_LENGTH][ANALYSIS_KERNEL_HALFLENGTH+1];
DiffusionFilter gaussianDiffusionFilter[NUM_VARIANCE][NUM_ANISOTROPY];
DiffusionFilter standardDiffusionFilter[NUM_TONE];
float noiseIntensity[NUM_TONE];

/// Definition of the Gabor filter. Called upon build of the lookup table.
/// The input arguments 'xFrequency' and 'yFrequency' are mapped from the valid frequencies of the analysis kernel.
static void buildGaborFilter(float xFrequency, float yFrequency, ThresholdFilter *out)
{
	float offset, sum = 0;
	int x, y, i, j;

	// First fill with a true Gabor function
	for(i = 0, x = -THRESHOLD_FILTER_HALFLENGTH;i < THRESHOLD_FILTER_LENGTH;i++, x++)
		for(j = 0, y = -THRESHOLD_FILTER_HALFLENGTH;j < THRESHOLD_FILTER_LENGTH;j++, y++)
		{
			// A frequency-dependant cosine term
			float cosFactor = cos(x*xFrequency + y*yFrequency);

			// Weighted by a fixed Gaussian function
			float gaussFactor = exp(-(x*x+y*y)/(2*GABOR_SIGMA*GABOR_SIGMA));

			sum += out->m[i][j] = cosFactor * gaussFactor;
		}

	// Then have mean be 0, making sure the DC of the resulting power spectrum is null
	offset = -sum / (THRESHOLD_FILTER_LENGTH*THRESHOLD_FILTER_LENGTH);
	for(i = 0;i < (THRESHOLD_FILTER_LENGTH*THRESHOLD_FILTER_LENGTH);i++)
		out->v[i] += offset;
}

/// Definition of the Gaussian filter. Called upon build of the lookup table.
/// The input arguments 'variance' and 'anisotropy' are given by calibration parameters 'sigma^2' and 'gamma'.
static void buildGaussianFilter(float variance, float anisotropy, DiffusionFilter *out)
{
	static const float COS_45 = SQRT2/2;
	static const float SIN_45 = SQRT2/2;
	static const float COS_135 = -SQRT2/2;
	static const float SIN_135 = SQRT2/2;
	float scale, sum = 0;
	int x, y, i, j;

	// First fill with a true anisotropic Gauss function
	for(i = 0, x = -DIFFUSION_FILTER_HALFLENGTH;i < DIFFUSION_FILTER_LENGTH;i++, x++)
		for(j = 0, y = -DIFFUSION_FILTER_HALFLENGTH;j < DIFFUSION_FILTER_LENGTH;j++, y++)
		{
			float u = (x * COS_45 + y * SIN_45) * anisotropy;
			float v = (x * COS_135 + y * SIN_135) / anisotropy;

			sum += out->m[i][j] = exp(-(u*u + v*v) / (2 * variance));
		}

	// Then make sure the sum is 1.0 over active coefficients (half of them, not counting the center)
	scale = 2.0f / (sum - 1.0f);		// We can always assume that the center coefficient was 1.0f
	for(i = 0;i < (DIFFUSION_FILTER_LENGTH*DIFFUSION_FILTER_LENGTH);i++)
		out->v[i] *= scale;
}

/// Builds proper diffusion filters from the data in the 'STANDARD_DIFFUSION_COEFFICIENT' table.
/// The input argument 'tone' is mapped to the valid range of the calibration data.
static void buildStandardDiffusionFilter(Tone *tone, DiffusionFilter *out)
{
	static const int c = DIFFUSION_FILTER_HALFLENGTH;
	int iTone = round((tone->gray - MIN_TONE) * (NUM_TONE-1) / (MAX_TONE - MIN_TONE));
	const float *lookup = (const float *) STANDARD_DIFFUSION_COEFFICIENT[iTone];

	memset(out->v, 0, DIFFUSION_FILTER_LENGTH * DIFFUSION_FILTER_LENGTH * sizeof(float));

	// Image traversal happens diagonally, hence the position of the four coefficients within the diffusion filter
	out->m[c + 1][c - 1] = lookup[0];		// top right
	out->m[c + 1][c + 0] = lookup[1];		// right
	out->m[c + 1][c + 1] = lookup[2];		// bottom right
	out->m[c + 0][c + 1] = lookup[3];		// bottom
}

/// An initialisable routine for reusable data. Must be called once.
/// IMPORTANT: will not be called by the 'get' functions to avoid conflicts with multiple threads.
void readyLookupTables()
{
	int i, j;

	for(i = 0;i < ANALYSIS_KERNEL_LENGTH;i++)
		for(j = 0;j <= ANALYSIS_KERNEL_HALFLENGTH;j++)
		{
			// Compute component-wise continuous frequencies
			float ux = i * 2*PI / ANALYSIS_KERNEL_LENGTH;
			float uy = j * 2*PI / ANALYSIS_KERNEL_LENGTH;

			buildGaborFilter(ux, uy, &gaborThresholdFilter[i][j]);
		}
	for(i = 0;i < NUM_VARIANCE;i++)
		for(j = 0;j < NUM_ANISOTROPY;j++)
		{
			// Recover continuous parameters used during calibration
			float variance = MIN_VARIANCE + i * (MAX_VARIANCE - MIN_VARIANCE) / (NUM_VARIANCE-1);
			float anisotropy = MIN_ANISOTROPY + j * (MAX_ANISOTROPY - MIN_ANISOTROPY) / (NUM_ANISOTROPY-1);

			buildGaussianFilter(variance, anisotropy, &gaussianDiffusionFilter[i][j]);
		}
	for(i = 0;i < NUM_TONE;i++)
	{
		Tone tone;
		tone.gray = MIN_TONE + i * (MAX_TONE - MIN_TONE) / (NUM_TONE-1);
		buildStandardDiffusionFilter(&tone, &standardDiffusionFilter[i]);
		noiseIntensity[i] = STANDARD_NOISE_MODULATION_INTENSITY[i];
	}
}

/// Retrieve calibration parameters.
const CalibrationParams *getCalibrationParams(LocalFrequencySignature *freq)
{
	int fx = freq->xWaveNumber, fy = freq->yWaveNumber;
	int iContrast = round(freq->contrast * (NUM_CONTRAST-1) / MAX_CONTRAST);

	ASSERT(fx >= 0 && fx < ANALYSIS_KERNEL_LENGTH && fy >= 0 && fy <= ANALYSIS_KERNEL_HALFLENGTH && iContrast >= 0 && iContrast < NUM_CONTRAST)
		fprintf(stderr, "getCalibrationParams(%d, %d, %d)\n", fx, fy, iContrast);

	return &CALIBRATION[fx][fy][iContrast];
}

/// Instantly obtain a frequency-specific Gabor convolution filter.
ThresholdFilter *getGaborFilter(LocalFrequencySignature *freq)
{
	int fx = freq->xWaveNumber, fy = freq->yWaveNumber;
	ASSERT(fx >= 0 && fx < ANALYSIS_KERNEL_LENGTH && fy >= 0 && fy <= ANALYSIS_KERNEL_HALFLENGTH)
		fprintf(stderr, "getGaborFilter(%d, %d)\n", fx, fy);

	return &gaborThresholdFilter[freq->xWaveNumber][freq->yWaveNumber];
}

/// Instantly obtain a specifically parameterised Gaussian diffusion filter.
DiffusionFilter *getGaussianFilter(float sigma, float gamma)
{
	float variance = sigma*sigma;
	float anisotropy = gamma;
	int iVariance = round((variance - MIN_VARIANCE) * (NUM_VARIANCE-1) / (MAX_VARIANCE - MIN_VARIANCE));
	int iAnisotropy = round((anisotropy - MIN_ANISOTROPY) * (NUM_ANISOTROPY-1) / (MAX_ANISOTROPY - MIN_ANISOTROPY));

	ASSERT(iVariance >= 0 && iVariance < NUM_VARIANCE && iAnisotropy >= 0 && iAnisotropy < NUM_ANISOTROPY)
		fprintf(stderr, "getGaussianFilter(%d, %d)\n", iVariance, iAnisotropy);

	return &gaussianDiffusionFilter[iVariance][iAnisotropy];
}

/// Instantly obtain a tone-specific variable-coefficient "standard" diffusion filter.
DiffusionFilter *getStandardFilter(Tone *tone)
{
	int iTone = round((tone->gray - MIN_TONE) * (NUM_TONE-1) / (MAX_TONE - MIN_TONE));

	ASSERT(iTone >= 0 && iTone < NUM_TONE)
		fprintf(stderr, "getStandardFilter(%d)\n", iTone);

	return &standardDiffusionFilter[iTone];
}

/// Instantly obtain a tone-specific noise threshold modulation intensity.
float getNoiseIntensity(Tone *tone)
{
	int iTone = round((tone->gray - MIN_TONE) * (NUM_TONE-1) / (MAX_TONE - MIN_TONE));

	ASSERT(iTone >= 0 && iTone < NUM_TONE)
		fprintf(stderr, "getNoiseIntensity(%d)\n", iTone);

	return noiseIntensity[iTone];
}
