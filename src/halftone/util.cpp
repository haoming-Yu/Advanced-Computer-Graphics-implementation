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


#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "halftone/util.h"
#include "halftone/analysis.h"
#include "halftone/lookup.h"


/*******
**  Local constants and macros
*******/

/// To avoid damageable boudary overflows, IMAGE_BORDER_PADDING is the halflength of the largest filter
#if ANALYSIS_KERNEL_LENGTH >= THRESHOLD_FILTER_LENGTH && ANALYSIS_KERNEL_LENGTH >= DIFFUSION_FILTER_LENGTH
	static const int IMAGE_BORDER_PADDING = ANALYSIS_KERNEL_HALFLENGTH;
#elif THRESHOLD_FILTER_LENGTH >= ANALYSIS_KERNEL_LENGTH && THRESHOLD_FILTER_LENGTH >= DIFFUSION_FILTER_LENGTH
	static const int IMAGE_BORDER_PADDING = THRESHOLD_FILTER_HALFLENGTH;
#else
	static const int IMAGE_BORDER_PADDING = DIFFUSION_FILTER_HALFLENGTH;
#endif

/// A macro that hides the column-major property of images and such.
#define MEMSHIFT(storage, x, y) ((storage)->mem + (x)*(storage)->step + (y))

/*******
**  Tone methods
*******/

/// Computes tone sum. Hides implementation details about the number of channels present.
void addTone(Tone *v1, Tone *v2, Tone *out)
{
	out->gray = v1->gray + v2->gray;
}
/// Computes tone difference. Hides implementation details about the number of channels present.
void subtractTone(Tone *v1, Tone *v2, Tone *out)
{
	out->gray = v1->gray - v2->gray;
}
/// Computes the linear interpolation between two tones.
/// Identified as 'Lerp' in paper's pseudocode.
void interpolateTone(Tone *v1, float s, Tone *v2, float t, Tone *out)
{
	out->gray = s*v1->gray + t*v2->gray;
}

/*******
**  Image methods
*******/

/// Reserves memory and fills image structure with useful information.
/// IMPORTANT: must be called before any use of an image.
void allocateImage(int width, int height, Image *out)
{
	out->width = width, out->height = height;
	out->step = height + 2*IMAGE_BORDER_PADDING;
	out->mem = (Tone *) malloc((width+2*IMAGE_BORDER_PADDING)*out->step*sizeof(Tone));

	// To simplify index offset computations, pre-shift the memory pointer
	out->mem = MEMSHIFT(out, IMAGE_BORDER_PADDING, IMAGE_BORDER_PADDING);
}
/// Makes the image permanent. Further modification of image data must be accompanied by other locks.
/// Implementation uses this call to extend the image data over its hidden padding border.
/// IMPORTANT: Must be called on input image before frequency analysis or before Gabor convolutions are performed.
void lockImage(Image *img)
{
	int x, y;
	for(x = -IMAGE_BORDER_PADDING;x < img->width + IMAGE_BORDER_PADDING;x++)
		for(y = -IMAGE_BORDER_PADDING;y < img->height + IMAGE_BORDER_PADDING;y++)
		{
			// Give a mirror copy of the image at outlying positions.
			int xMirror = x < 0 ? -x : x >= img->width ? 2*(img->width-1)-x : x;
			int yMirror = y < 0 ? -y : y >= img->height ? 2*(img->height-1)-y : y;
			*MEMSHIFT(img, x, y) = *MEMSHIFT(img, xMirror, yMirror);
		}
}
/// Releases internal memory handled by image. Should be called to avoid memory leaks.
void freeImage(Image *img)
{
	if(img->mem)
		free(MEMSHIFT(img, -IMAGE_BORDER_PADDING, -IMAGE_BORDER_PADDING));
	img->mem = 0;
}
/// Sets all visible tone information to 0. Used to reset the error information before diffusion.
void clearImage(Image *img)
{
	memset(img->mem, 0, img->width*img->step*sizeof(Tone));
}

/// Interface between the common three-channel 8-bit RGB color profile and the Tone structure.
/// Values of the input argument have a valid range of [0, 255].
/// Implementation stores color intensity as a floating-point value in [0.0, 1.0].
void setPixelProfile(Image *img, int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	MEMSHIFT(img, x, y)->gray = MIN_TONE + (r+g+b) * (MAX_TONE - MIN_TONE) / (3*255);
}
/// Interface between the common three-channel 8-bit RGB color profile and the Tone structure.
/// Values of the input argument have a valid range of [0, 255].
/// Implementation stores color intensity as a floating-point value in [0.0, 1.0].
unsigned char getRed(Image *img, int x, int y)
{
	return (unsigned char) round(255 * (MEMSHIFT(img, x, y)->gray - MIN_TONE) / (MAX_TONE - MIN_TONE));
}
/// Interface between the common three-channel 8-bit RGB color profile and the Tone structure.
/// Values of the input argument have a valid range of [0, 255].
/// Implementation stores color intensity as a floating-point value in [0.0, 1.0].
unsigned char getGreen(Image *img, int x, int y)
{
	return getRed(img, x, y);
}
/// Interface between the common three-channel 8-bit RGB color profile and the Tone structure.
/// Values of the input argument have a valid range of [0, 255].
/// Implementation stores color intensity as a floating-point value in [0.0, 1.0].
unsigned char getBlue(Image *img, int x, int y)
{
	return getRed(img, x, y);
}

/// The general purpose middle-level method to access image data. Use the MEMSHIFT macro for lower-level operations.
/// Returns a pointer to image memory at specified position inside boundary.
Tone *pixelPointer(Image *img, int x, int y)
{
	return MEMSHIFT(img, x, y);
}

/*******
**  FrequencyContent methods
*******/

/// Method to prepare and access the frequency content structure. Analogous to that for images.
void allocateFrequencyContent(int width, int height, FrequencyContent *out)
{
	out->step = height;
	out->mem = (LocalFrequencySignature *) malloc(width*height*sizeof(LocalFrequencySignature));
}
/// Releases internal memory used by frequency content. Should be called to avoid memory leaks.
void freeFrequencyContent(FrequencyContent *freq)
{
	if(freq->mem)
		free(freq->mem);
	freq->mem = 0;
}
/// Interface method towards the intuitive frequency-orientation-contrast description of the local frequency signature
void setFrequencyProfile(FrequencyContent *freqContent, int x, int y, double frequency, double orientation, double contrast)
{
	LocalFrequencySignature *out = MEMSHIFT(freqContent, x, y);

	double waveNumber = frequency * ANALYSIS_KERNEL_LENGTH / (2*PI);
	int xNumber = round(waveNumber * cos(orientation));
	int yNumber = round(waveNumber * sin(orientation));

	// Avoid unmatching but equivalent frequencies
	if(yNumber < 0)
		xNumber = -xNumber, yNumber = -yNumber;

	// Constrain wave numbers to analysis's valid bandwidth
	if(yNumber == 0 && fabs((double)xNumber) >= ANALYSIS_KERNEL_HALFLENGTH)
		xNumber = -ANALYSIS_KERNEL_HALFLENGTH;
	if(yNumber > ANALYSIS_KERNEL_HALFLENGTH)
		yNumber = ANALYSIS_KERNEL_HALFLENGTH;
	if(xNumber > ANALYSIS_KERNEL_HALFLENGTH-1)
		xNumber = ANALYSIS_KERNEL_HALFLENGTH-1;
	if(xNumber < -ANALYSIS_KERNEL_HALFLENGTH)
		xNumber = -ANALYSIS_KERNEL_HALFLENGTH;

	// The computed wave numbers have to be in the range [0, ANALYSIS_KERNEL_LENGTH-1]
	out->xWaveNumber = xNumber < 0 ? xNumber + ANALYSIS_KERNEL_LENGTH : xNumber;
	out->yWaveNumber = yNumber;

	// Contrast information is not scaled, but is contrained to the range of 0 to MAX_CONTRAST
	out->contrast = contrast < 0 ? 0 : contrast > MAX_CONTRAST ? MAX_CONTRAST : (float) contrast;
}
/// Interface method towards the intuitive frequency-orientation-contrast description of the local frequency signature
double getFrequency(FrequencyContent *freqContent, int x, int y)
{
	LocalFrequencySignature *freq = MEMSHIFT(freqContent, x, y);
	double xNumber = freq->xWaveNumber < ANALYSIS_KERNEL_HALFLENGTH ? freq->xWaveNumber : freq->xWaveNumber - ANALYSIS_KERNEL_LENGTH;
	double yNumber = freq->yWaveNumber;
	return sqrt(xNumber*xNumber + yNumber*yNumber) * (2*PI) / ANALYSIS_KERNEL_LENGTH;
}
/// Interface method towards the intuitive frequency-orientation-contrast description of the local frequency signature
double getOrientation(FrequencyContent *freqContent, int x, int y)
{
	LocalFrequencySignature *freq = MEMSHIFT(freqContent, x, y);
	double xNumber = freq->xWaveNumber < ANALYSIS_KERNEL_HALFLENGTH ? freq->xWaveNumber : freq->xWaveNumber - ANALYSIS_KERNEL_LENGTH;
	double yNumber = freq->yWaveNumber;
	return atan2(yNumber, xNumber);
}
/// Interface method towards the intuitive frequency-orientation-contrast description of the local frequency signature
double getContrast(FrequencyContent *freqContent, int x, int y)
{
	return MEMSHIFT(freqContent, x, y)->contrast;
}

/// The general purpose middle-level method to access frequency data. Use the MEMSHIFT macro for lower-level operations.
/// Returns a pointer to the internal memory of the 'FrequencyContent' structure at specified position inside boundary.
LocalFrequencySignature *frequencyPointer(FrequencyContent *freqContent, int x, int y)
{
	return MEMSHIFT(freqContent, x, y);
}

/*******
**  DiffusionFilter methods
*******/

/// Flips the diffusion coefficients with respect to the symmetry axis of the raster traversal scheme.
/// Implementation gives a reflexion axis oriented at 45 degrees, a transposition that is.
void alignDiffusionFilter(DiffusionFilter *filter, RasterTraversalIterator *traversal, DiffusionFilter *out)
{
	int i, j;
	if(traversal->direction < 0)
	{
		// reflexion by "y = x" axis
		if(filter == out)
			for(i = 0;i < DIFFUSION_FILTER_LENGTH;i++) for(j = 0;j < i;j++)
			{
				float temp = out->m[i][j];
				out->m[i][j] = filter->m[j][i];
				filter->m[j][i] = temp;
			}
		else
			for(i = 0;i < DIFFUSION_FILTER_LENGTH;i++) for(j = 0;j < DIFFUSION_FILTER_LENGTH;j++)
				out->m[i][j] = filter->m[j][i];
	}
}
/// Computes the linear interpolation between each corresponding elements of two diffusion filters.
/// Identified as 'Lerp' in paper's pseudocode.
void interpolateDiffusionFilter(DiffusionFilter *v1, float s, DiffusionFilter *v2, float t, DiffusionFilter *out)
{
	int i;
	for(i = 0;i < (DIFFUSION_FILTER_LENGTH*DIFFUSION_FILTER_LENGTH);i++)
		out->v[i] = s*v1->v[i] + t*v2->v[i];
}

/*******
**  RasterTraversalIterator methods
*******/

/// Positions raster traversal iterator to origin and sets its boundaries.
/// Implementation also computes the number of positions remaining to explore.
void initRasterTraversal(RasterTraversalIterator *traversal, int width, int height)
{
	traversal->x = traversal->y = 0;
	traversal->width = width, traversal->height = height;
	traversal->direction = 1;
	traversal->count = -width*height;
}
/// Sets raster traversal iterator to next pixel position according to designed traversal order.
/// Returns zero when there are no more positions to explore. Iterating further has undefined behavior.
/// Implementation travels diagonally and alternates direction from up-right to down-left whenever a border is hit.
/// IMPORTANT: changing the traversal order requires recalibration of the diffusion coefficients.
int iterateRasterTraversal(RasterTraversalIterator *traversal)
{
	traversal->x += traversal->direction;
	traversal->y -= traversal->direction;

	// When out of range, undo movement, slide towards the bottom-right corner, and change travel direction
	if(traversal->x >= traversal->width)
		traversal->x--, traversal->y += 2, traversal->direction *= -1;
	else if(traversal->y >= traversal->height)
		traversal->y--, traversal->x += 2, traversal->direction *= -1;
	else if(traversal->x < 0)
		traversal->x++, traversal->direction *= -1;
	else if(traversal->y < 0)
		traversal->y++, traversal->direction *= -1;

	return ++traversal->count;
}

/*******
**  General-purpose methods
*******/

/// Spreads the error term on the unexplored neighboring pixels.
/// The diffusion filters are guaranteed to have a total weight of 1.0 over their valid range.
/// Implementation uses image's border padding to avoid boundary checking.
void distributeError(Image *err, int xFocus, int yFocus, DiffusionFilter *filter)
{
	float eFocus = MEMSHIFT(err, xFocus, yFocus)->gray;
	int xStart = xFocus - DIFFUSION_FILTER_HALFLENGTH;
	int yStart = yFocus - DIFFUSION_FILTER_HALFLENGTH;
	int i, j;

	for(i = 0;i < DIFFUSION_FILTER_LENGTH;i++)
		for(j = 0;j < DIFFUSION_FILTER_LENGTH;j++)
			MEMSHIFT(err, xStart + i, yStart + j)->gray += eFocus * filter->m[i][j];
}

/// Computes the element-wise product of image and filter, and outputs the sum.
/// Semantically, the result of that "tone * float" product is also a tone.
/// Implementation uses image's border padding to avoid boundary checking.
void computeLocalConvolution(Image *img, int xFocus, int yFocus, ThresholdFilter *filter, Tone *out)
{
	int xStart = xFocus - THRESHOLD_FILTER_HALFLENGTH;
	int yStart = yFocus - THRESHOLD_FILTER_HALFLENGTH;
	float sum = 0;
	int i, j;

	for(i = 0;i < THRESHOLD_FILTER_LENGTH;i++)
		for(j = 0;j < THRESHOLD_FILTER_LENGTH;j++)
			sum += MEMSHIFT(img, xStart + i, yStart + j)->gray * filter->m[i][j];

	out->gray = sum;
}

/// Noise function used for thresholding in the absence of significant frequency content.
/// Implementation relies on stdlib's 'rand()' function to produce incoherent white noise.
void noise(float intensity, Tone *out)
{
	out->gray = intensity * (float)random(-0.5, 0.5);
}

/// Compares given tone to a modulated threshold and outputs black if darker, white otherwise.
/// Implementation defines black as 0 and uses a standard Gray50% threshold (before modulation).
/// IMPORTANT: modulation is added to the tone rather than the threshold. Signs must be handled carefully.
void quantize(Tone *contone, Tone *thresholdModulation, Tone *out)
{
	out->gray = contone->gray + thresholdModulation->gray < MID_TONE ? MIN_TONE : MAX_TONE;
}

