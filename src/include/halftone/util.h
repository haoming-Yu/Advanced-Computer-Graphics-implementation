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


#ifndef _SAED__UTIL_H_
#define _SAED__UTIL_H_

/** \file util.h
 *
 *  A header containing declarations of the principal structures and functions used all files.
 *  Contains some functions that are declared in saeh.h as well.
 *  Most important structureAwareErrorDiffusion method makes extensive use of the functions in util.h.
 */

/** \file	util.c */
/** \typedef	Tone */
/** \typedef	LocalFrequencySignature */
/** \typedef	Image */
/** \typedef	FrequencyContent */
/** \typedef	ThresholdFilter */
/** \typedef	DiffusionFilter */
/** \typedef	RasterTraversalIterator */
/** \fn		subtractTone */
/** \fn		interpolateTone */
/** \fn		allocateImage */
/** \fn		lockImage */
/** \fn		freeImage */
/** \fn		clearImage */
/** \fn		pixelPointer */
/** \fn		allocateFrequencyContent */
/** \fn		freeFrequencyContent */
/** \fn		frequencyPointer */
/** \fn		alignDiffusionFilter */
/** \fn		interpolateDiffusionFilter */
/** \fn		initRasterTraversal */
/** \fn		iterateRasterTraversal */
/** \fn		distributeError */
/** \fn		computeLocalConvolution */
/** \fn		noise */
/** \fn		quantize */
/** \def	DEBUG	\brief Enable validity checking. Slower. In principle, this flag should be left inactive. */

// Set this flag to force calibration validity checking every time data is retrieved from the lookup tables.
//#define DEBUG
#include <math.h>
#ifdef DEBUG
	#include <stdio.h>
	#define ASSERT(x) if(!(x))
#else
	#define ASSERT(x) if(0)
#endif

// Some useful constants.
#define PI		3.141592654
#define SQRT2		1.414213562

// Some value range conventions. Anything goes as long as MAX_TONE > MIN_TONE.
#define MIN_TONE		0.0f
#define MAX_TONE		1.0f
#define MID_TONE		((MAX_TONE + MIN_TONE) / 2)
#define MAX_CONTRAST		((MAX_TONE - MIN_TONE) / 2)

// Filter sizes used for Gabor convolution (11x11) and Gaussian diffusion patterns (5x5)
// These values are empirical design choices aimed at satisfying quality and speed.
#define THRESHOLD_FILTER_HALFLENGTH		5
#define DIFFUSION_FILTER_HALFLENGTH		2
#define THRESHOLD_FILTER_LENGTH			(2 * THRESHOLD_FILTER_HALFLENGTH + 1)
#define DIFFUSION_FILTER_LENGTH			(2 * DIFFUSION_FILTER_HALFLENGTH + 1)

/// Fundamental data types for tone computations.
/// Implementation supports only single-channel tones.
typedef struct
{
	float gray;
} Tone;
/// The local frequency signature to be computed at each pixel position
typedef struct
{
	int xWaveNumber, yWaveNumber;
	float contrast;
} LocalFrequencySignature;

/// Image structure. Assumed to be filled externally. See "example.c" for specific file handling.
/// Padding is used to avoid boundary overflow. Variable 'step' replaces actual dimensions when computing index offset.
/// IMPORTANT: images store their data linearly in column-major order. 'offset = x*step + y'
typedef struct
{
	Tone *mem;
	int width, height, step;
} Image;
/// Image-like structure that maps pixels to their local frequency profile.
/// Assumed to be filled externally. See "example.c" for specific file handling.
/// Variable 'step' replaces actual dimensions when computing index offset, although no padding is used in implementation.
/// IMPORTANT: data is stored linearly in column-major order. 'offset = x*step + y'
typedef struct
{
	LocalFrequencySignature *mem;
	int step;
} FrequencyContent;

/// Specialized filter for the threshold step of our method.
/// Provides a double interface, to be accessible by indices or as a whole.
typedef union
{
	float v[THRESHOLD_FILTER_LENGTH*THRESHOLD_FILTER_LENGTH];
	float m[THRESHOLD_FILTER_LENGTH][THRESHOLD_FILTER_LENGTH];
} ThresholdFilter;
/// Specialized filter for the diffusion step of our method.
/// Provides a double interface, to be accessible by indices or as a whole.
typedef union
{
	float v[DIFFUSION_FILTER_LENGTH*DIFFUSION_FILTER_LENGTH];
	float m[DIFFUSION_FILTER_LENGTH][DIFFUSION_FILTER_LENGTH];
} DiffusionFilter;

/// Retains the information about the serial traversal of the input image pixels.
/// The variable 'direction' is needed to achieve boustrophedon marching.
typedef struct
{
	int x, y, direction;	// 'direction' is either 1 or -1
	int width, height;
	int count;
} RasterTraversalIterator;

// Math method entries -- trying to avoid a ton of C/C++ and Windows/Linux conflicts
#define round(x)	( (int) floor( 0.5+(x) ) )
#define random(x, y)	( (double) ( (x) + ((y)-(x)) * rand() / (double) RAND_MAX ) )
#define sqrt(x)		( (double) sqrt( (double)(x) ) )
#define atan2(y, x)	( (double) atan2( (double)(y), (double)(x) ) )
// Tone methods
void addTone(Tone *v1, Tone *v2, Tone *out);
void subtractTone(Tone *v1, Tone *v2, Tone *out);
void interpolateTone(Tone *v1, float s, Tone *v2, float t, Tone *out);
// Image methods
void allocateImage(int width, int height, Image *out);
void lockImage(Image *img);
void freeImage(Image *img);
void clearImage(Image *img);
Tone *pixelPointer(Image *img, int x, int y);
// FrequencyContent methods
void allocateFrequencyContent(int width, int height, FrequencyContent *out);
void freeFrequencyContent(FrequencyContent *freq);
LocalFrequencySignature *frequencyPointer(FrequencyContent *freq, int x, int y);
// DiffusionFilter methods
void alignDiffusionFilter(DiffusionFilter *filter, RasterTraversalIterator *traversal, DiffusionFilter *out);
void interpolateDiffusionFilter(DiffusionFilter *v1, float s, DiffusionFilter *v2, float t, DiffusionFilter *out);
// RasterTraversalIterator methods
void initRasterTraversal(RasterTraversalIterator *traversal, int width, int height);
int iterateRasterTraversal(RasterTraversalIterator *traversal);
// General-purpose methods
void distributeError(Image *err, int xFocus, int yFocus, DiffusionFilter *filter);
void computeLocalConvolution(Image *img, int xFocus, int yFocus, ThresholdFilter *filter, Tone *out);
void noise(float intensity, Tone *out);
void quantize(Tone *contone, Tone *thresholdModulation, Tone *out);

#endif	// !defined(_SAED__UTIL_H_)
