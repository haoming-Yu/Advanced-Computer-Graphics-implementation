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


#ifndef _SAED__SAED_H_
#define _SAED__SAED_H_

/**
 *  \file saed.h
 *  The global header that declares all methods that do not rely on internal implementation details.
 *  This is the only header that should be declared in the file where main() is.
 */

/**  \file	saed.c */
/**  \fn	structureAwareErrorDiffusion */
/**  \file	util.c */
/**  \fn	allocateImage */
/**  \fn	lockImage */
/**  \fn	freeImage */
/**  \fn	setPixelProfile */
/**  \fn	getRed */
/**  \fn	getGreen */
/**  \fn	getBlue */
/**  \fn	allocateFrequencyContent */
/**  \fn	freeFrequencyContent */
/**  \fn	setFrequencyProfile */
/**  \fn	getFrequency */
/**  \fn	getOrientation */
/**  \fn	getContrast */
/**  \file	analysis.c */
/**  \fn	localFrequencyAnalysis */
/**  \fn	readyFrequencyAnalysis */
/**  \file	lookup.c */
/**  \fn	readyLookupTables */

/// Fundamental data types for tone computations.
/// Implementation supports only single-channel tones.
struct s_Tone;
typedef struct s_Tone Tone;

/// The local frequency signature to be computed at each pixel position
struct s_LocalFrequencySignature;
typedef struct s_LocalFrequencySignature LocalFrequencySignature;

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

// From saed.c
void structureAwareErrorDiffusion(Image *img, FrequencyContent *freqContent, Image *out);

// From util.c
void allocateImage(int width, int height, Image *out);
void lockImage(Image *img);
void freeImage(Image *img);
void setPixelProfile(Image *img, int x, int y, unsigned char r, unsigned char g, unsigned char b);
unsigned char getRed(Image *img, int x, int y);
unsigned char getGreen(Image *img, int x, int y);
unsigned char getBlue(Image *img, int x, int y);
void allocateFrequencyContent(int width, int height, FrequencyContent *out);
void freeFrequencyContent(FrequencyContent *freq);
void setFrequencyProfile(FrequencyContent *freqContent, int x, int y, double frequency, double orientation, double contrast);
double getFrequency(FrequencyContent *freqContent, int x, int y);
double getOrientation(FrequencyContent *freqContent, int x, int y);
double getContrast(FrequencyContent *freqContent, int x, int y);

// From analysis.c
void localFrequencyAnalysis(Image *img, int x, int y, FrequencyContent *out);
void readyFrequencyAnalysis();

// From lookup.c
void readyLookupTables();

#endif	// !defined(_SAED__SAED_H_)
