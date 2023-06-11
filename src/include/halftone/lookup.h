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


#ifndef _SAED__LOOKUP_H_
#define _SAED__LOOKUP_H_

/**
 *  \file lookup.h
 *
 *  Declarations that pertain mainly to lookup tables.
 *  Contains specific implementation details that should remain transparent to other files than 'lookup.c'.
 */

/** \file	lookup.c */
/** \def	GABOR_SIGMA	\brief Gives a smooth yet discriminative Gabor filter */
/** \typedef	CalibrationParams */
/** \fn		readyLookupTables */
/** \fn		getCalibrationParams */
/** \fn		getGaborFilter */
/** \fn		getGaussianFilter */
/** \fn		getStandardFilter */
/** \fn		getNoiseIntensity */

// Specific design choices
#define GABOR_SIGMA		1.6f			// Gives a smooth yet discriminative Gabor filter
#define NUM_TONE		256
#define NUM_CONTRAST		128
#define NUM_VARIANCE		256
#define NUM_ANISOTROPY		256

/// The set of frequency-calibrated parameters that define the behavior of our method.
/// 'beta' is a strength multiplicative factor for the Gabor convolution.
/// 'sigma' is the standard deviation of the Gaussian filter, that is, 'variance = sigma*sigma'
/// 'a' is the anisotropy traversal-parallel stretching, traversal-perpendicular anti-stretching factor of the Gaussian filter.
/// 'w' is the linear interpolation control parameter, 0 = standard behavior, 1 = frequency-sensitive behavior.
typedef struct
{
	float beta, sigma, a, w;
} CalibrationParams;

void readyLookupTables();
const CalibrationParams *getCalibrationParams(LocalFrequencySignature *freq);
ThresholdFilter *getGaborFilter(LocalFrequencySignature *freq);
DiffusionFilter *getGaussianFilter(float sigma, float a);
DiffusionFilter *getStandardFilter(Tone *tone);
float getNoiseIntensity(Tone *tone);

extern const float STANDARD_DIFFUSION_COEFFICIENT[NUM_TONE][4];
extern const float STANDARD_NOISE_MODULATION_INTENSITY[NUM_TONE];
extern const CalibrationParams ADAPTIVE_PARAMETERS_CALIBRATION[ANALYSIS_KERNEL_LENGTH][ANALYSIS_KERNEL_HALFLENGTH+1][NUM_CONTRAST];
extern const float MIN_PARAM_SIGMA;
extern const float MAX_PARAM_SIGMA;
extern const float MIN_PARAM_A;
extern const float MAX_PARAM_A;

// Static calibration data.
// All visual calibrations have been interpolated offline and placed in header files.
#ifndef STANDARD_CALIBRATION_HEADER
#define STANDARD_CALIBRATION_HEADER "halftone/calib/std.h"
#endif

#ifndef ADAPTIVE_CALIBRATION_HEADER
	#ifndef KERNEL_SIZE
		#include "analysis.h"
	#endif

	#if KERNEL_SIZE == 4
		#define ADAPTIVE_CALIBRATION_HEADER "calib/ker4.h"
	#elif KERNEL_SIZE == 8
		#define ADAPTIVE_CALIBRATION_HEADER "calib/ker8.h"
	#elif KERNEL_SIZE == 16
		#define ADAPTIVE_CALIBRATION_HEADER "halftone/calib/ker16.h"
	#elif KERNEL_SIZE == 32
		#define ADAPTIVE_CALIBRATION_HEADER "calib/ker32.h"
	#else
		#error "No calibration data is available. Please define ADAPTIVE_CALIBRATION_HEADER to a calibration file path."
	#endif
#endif

// Manual calibration data.
// IMPORTANT: Unused in this implementation. Left for reference only.

// Below is the calibration used to generate the best-looking natural images.
// Please take note that the calibration used for the artificial images (patches, ramps),
// are different in order to achieve best quality.
// Calibration used to produce images with the best PSNR and MSSIM objective quality measures use a third set of tuning.
// To avoid confusion, these two additional sets of calibration were left out of the code.
// Please contact the authors of the paper to obtain them.

#if 0
// Convolution strength for threshold filtering. Values must be continuous.
// In the main calibration, that for natural images, beta parameter was found empirically for key frequency signatures.
// Frequencies were 0.125*pi, 0.25*pi, 0.375*pi, 0.5*pi, 0.625*pi, 0.75*pi, 0.875*pi, 1.0*pi.
// All orientations share the same strength.
// Contrasts were 5/255, 12/255, 25/255, 51/255, 76/255, 102/255, 127.5/255.
// Interpolation was done bilinearly.
const float MANUAL_CONVOLUTION_FREQ[8] = { 0.392699f, 0.785398f, 1.178097f, 1.570796f, 1.963495f, 2.356194f, 2.748894f, 3.141593f };
const float MANUAL_CONVOLUTION_CONT[7] = { 0.019531f, 0.046875f, 0.097656f, 0.199219f, 0.296875f, 0.398438f, 0.5f };
const float MANUAL_CONVOLUTION_CALIB[8][7] = {
	0.185000f, 0.105000f, 0.090000f, 0.155000f, 0.125000f, 0.035000f, 0.000000f,
	0.370000f, 0.210000f, 0.180000f, 0.310000f, 0.250000f, 0.070000f, 0.000000f,
	0.465000f, 0.205000f, 0.190000f, 0.325000f, 0.295000f, 0.125000f, 0.000000f,
	0.560000f, 0.200000f, 0.200000f, 0.340000f, 0.340000f, 0.180000f, 0.000000f,
	0.620000f, 0.270000f, 0.200000f, 0.320000f, 0.355000f, 0.160000f, 0.000000f,
	0.680000f, 0.340000f, 0.200000f, 0.300000f, 0.370000f, 0.140000f, 0.000000f,
	0.605000f, 0.325000f, 0.255000f, 0.265000f, 0.285000f, 0.120000f, 0.005000f,
	0.520000f, 0.310000f, 0.310000f, 0.230000f, 0.200000f, 0.100000f, 0.010000f
};

// Diffusion filter calibration. Values must be continuous. Some symmetries apply.
// (sigma, a) pairs were found empirically for key frequency signatures.
// Frequencies were 0.375*pi, 0.5*pi, 0.625*pi, 0.78125*pi;  (4 keys)
// Orientations were pi/4, atan(2.0), pi/2, pi-atan(2.0), 3*pi/4;  (5 keys + two symmetry axes)
// Contrasts were 0, 20/255, 60/255, 100/255, 127/255.  (5 keys)
const float MANUAL_DIFFUSION_FREQ[4] = { 1.178097f, 1.570796f, 1.963495f, 2.454369f };
const float MANUAL_DIFFUSION_ORI[5] = { 0.785398f, 1.107149f, 1.570796f, 2.034444f, 2.356194f };
const float MANUAL_DIFFUSION_CONT[5] = { 0.000000f, 0.078431f, 0.235294f, 0.392157f, 0.498039f };
const struct { float sigma, a; } MANUAL_DIFFUSION_CALIB[4][5][5] = {
	1.386948f,0.683486f,  0.999003f,0.800000f,  0.767940f,0.946100f,  0.654560f,1.088662f,  0.665942f,1.206045f,
	1.386948f,0.683486f,  1.153550f,0.923760f,  0.814880f,1.003929f,  0.667029f,1.109400f,  0.624709f,1.131371f,
	1.386948f,0.683486f,  1.278120f,1.055009f,  0.768555f,0.984732f,  0.646343f,1.098884f,  0.633326f,1.131371f,
	1.386948f,0.683486f,  0.964520f,0.831800f,  0.697972f,0.963087f,  0.619810f,1.069045f,  0.637591f,1.154701f,
	1.386948f,0.683486f,  0.560307f,0.759326f,  0.541568f,0.827340f,  0.579058f,0.977356f,  0.499939f,0.880771f,
	1.386948f,0.683486f,  1.116755f,0.963087f,  0.736540f,0.952786f,  0.670308f,1.131371f,  0.736226f,1.315192f,
	1.386948f,0.683486f,  1.173390f,1.011929f,  0.749957f,0.970143f,  0.680595f,1.148733f,  0.615141f,1.098884f,
	1.386948f,0.683486f,  1.189362f,1.055009f,  0.723822f,0.963087f,  0.599563f,1.041511f,  0.593164f,1.059626f,
	1.386948f,0.683486f,  1.116920f,0.894427f,  0.670818f,1.000000f,  0.608046f,1.078720f,  0.675471f,1.264911f,
	1.386948f,0.683486f,  0.708291f,0.715542f,  0.544601f,0.771589f,  0.502260f,0.897235f,  0.553339f,0.988483f,
	1.386948f,0.683486f,  0.975360f,0.865181f,  0.749852f,0.988483f,  0.687722f,1.160762f,  0.768431f,1.382190f,
	1.386948f,0.683486f,  1.116920f,0.894427f,  0.718634f,0.956183f,  0.717559f,1.166920f,  0.664599f,1.219989f,
	1.386948f,0.683486f,  1.043829f,0.977356f,  0.729804f,1.016001f,  0.590331f,1.083657f,  0.574725f,1.055009f,
	1.386948f,0.683486f,  1.061033f,0.888889f,  0.614349f,1.059626f,  0.732264f,1.425393f,  0.650879f,1.403293f,
	1.386948f,0.683486f,  1.007187f,0.905822f,  0.599687f,0.908739f,  0.540066f,0.984732f,  0.594320f,1.083657f,
	1.386948f,0.683486f,  1.194567f,1.059626f,  0.870098f,1.264911f,  0.769621f,1.403293f,  0.732702f,1.525540f,
	1.386948f,0.683486f,  1.104571f,1.088662f,  0.753553f,1.011929f,  0.687722f,1.160762f,  0.672220f,1.192570f,
	1.386948f,0.683486f,  1.122547f,1.078720f,  0.673454f,1.003929f,  0.599239f,1.114773f,  0.630633f,1.173177f,
	1.386948f,0.683486f,  1.024414f,0.883452f,  0.499872f,0.923760f,  0.604998f,1.192570f,  0.570398f,1.264911f,
	1.386948f,0.683486f,  1.116920f,0.894427f,  0.663093f,0.988483f,  0.581406f,1.024295f,  0.614335f,1.142857f,
};

// Adaptive linear transition from standard behavior to structure-sensitive behavior. Values must be continuous.
// w parameter was computed from a very simple formula.
//
// If frequency < 2*pi/8, then p1 = 0.
// If frequency > 3*pi/8, then p1 = 1.
// Otherwise, p1 = (frequency - 2*pi/8) / (3*pi/8 - 2*pi/8).
//
// If contrast < 0.02, then p2 = 0.
// If contrast > 0.05, then p2 = 1.
// Otherwise, p2 = (contrast - 0.02) / (0.05 - 0.02).
//
// w = 1 - p1*p2.
#endif

#endif	// !defined(_SAED__LOOKUP_H_)
