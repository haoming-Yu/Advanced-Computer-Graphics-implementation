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

#include "halftone/util.h"
#include "halftone/analysis.h"
#include "halftone/lookup.h"

/// The method presented in the paper.
/// See pseudo-code in Appendix A of the paper for a simplified layout of this method.
void structureAwareErrorDiffusion(Image *img, FrequencyContent *freqContent, Image *out)
{
	RasterTraversalIterator traversal;
	DiffusionFilter modifiedDiffusionFilter;
	Tone gaborThreshold, noiseThreshold, modulatedThreshold, corrected;

	Image err; // Array of quantization error values
	allocateImage(img->width, img->height, &err);
	clearImage(&err);

	initRasterTraversal(&traversal, img->width, img->height);
	do {
		Tone *imgptr = pixelPointer(img, traversal.x, traversal.y);
		Tone *outptr = pixelPointer(out, traversal.x, traversal.y);
		Tone *errptr = pixelPointer(&err, traversal.x, traversal.y);
		LocalFrequencySignature *freq = frequencyPointer(freqContent, traversal.x, traversal.y);

		// Retrieve threshold and diffusion filter from lookup tables
		const CalibrationParams *params = getCalibrationParams(freq);
		ThresholdFilter *gaborFilter = getGaborFilter(freq);
		DiffusionFilter *gaussianFilter = getGaussianFilter(params->sigma, params->a);
		DiffusionFilter *standardFilter = getStandardFilter(imgptr);
		computeLocalConvolution(img, traversal.x, traversal.y, gaborFilter, &gaborThreshold);
		noise(getNoiseIntensity(imgptr), &noiseThreshold);

		interpolateTone(&gaborThreshold, params->beta, &noiseThreshold, params->w, &modulatedThreshold);
		interpolateDiffusionFilter(gaussianFilter, 1 - params->w, standardFilter, params->w, &modifiedDiffusionFilter);
		alignDiffusionFilter(&modifiedDiffusionFilter, &traversal, &modifiedDiffusionFilter);

		// Perform error diffusion
		addTone(imgptr, errptr, &corrected);
		quantize(&corrected, &modulatedThreshold, outptr);
		subtractTone(&corrected, outptr, errptr);
		distributeError(&err, traversal.x, traversal.y, &modifiedDiffusionFilter);
	}
	while(iterateRasterTraversal(&traversal));

	freeImage(&err);
}
