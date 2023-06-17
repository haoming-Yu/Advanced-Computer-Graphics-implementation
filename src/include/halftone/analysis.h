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


#ifndef _SAED__ANALYSIS_H_
#define _SAED__ANALYSIS_H_

/** \file analysis.h
 *
 *  A header crucial to any middle- to low-level manipulations of the 'LocalFrequencySignature' structure.
 */
/** \file analysis.c */

#define FASTEST_ANALYSIS		4	// Runs very fast even on CPU
#define FAST_ANALYSIS			8	// Gives reasonable accuracy at good speed
#define ACCURATE_ANALYSIS		16	// Probably the best quality/speed tradeoff.
#define MOST_ACCURATE_ANALYSIS		32	// Better accuracy than depth=4, very similar to depth=6 and up

// Implementation uses a kernel size of 16. All images shown in the paper were produced with that size.
#ifndef KERNEL_SIZE
#define KERNEL_SIZE			ACCURATE_ANALYSIS
#endif

// The width and height of the analysis matrixs. Powers of 2 are forced for faster computations.
#define ANALYSIS_KERNEL_LENGTH		KERNEL_SIZE

// The halflength is used to position the center of the windowing function.
// Note that by contrast to the threshold and diffusion filters, the analysis kernel has even length.
// However, the consequences are minimal since the windowing functions are properly centered on the (0, 0) pixel.
#define ANALYSIS_KERNEL_HALFLENGTH	(ANALYSIS_KERNEL_LENGTH / 2)

#endif	// !defined(_SAED__ANALYSIS_H_)

