#include"halftone/halftone.h"
using namespace cv;
int buildFrequencyContent(Image *img, FrequencyContent *out)
{
	int i, j;  
	readyFrequencyAnalysis(); 
	allocateFrequencyContent(img->width, img->height, out);
	// For reasons of compatibility, a very simple serial build.
	// Please contact paper's authors if interested in a parallel implementation.	
	for(i = 0;i < img->width;i++) for(j = 0;j < img->height;j++)
	{
		//if(!j) printf("\rBuilding frequency content ... %d%%", (i*img->height + j) * 100 / (img->width*img->height));
		localFrequencyAnalysis(img, i, j, out);
	}	
	//printf("\rBuilding frequency content ... 100%%\n");
	return 0;
}
void cvHalftone(cv::Mat *_in, cv::Mat* _out)
{
	CvMat cvmat_in = cvMat(*_in);
	CvMat cvmat_out = cvMat(*_out);
	CvMat* in = &cvmat_in,*out = &cvmat_out;
	static int ready = 0;
	int width = in->cols, height = in->rows;
	Image img, bitone;
	FrequencyContent freqContent;
	int i, j;

	// Init -- may be a little long, so better avoid repeating
	if(!ready)
	{
		readyLookupTables();
		ready = 1;
	}

	// Get image
	allocateImage(width, height, &img);
	allocateImage(width, height, &bitone);
	for(i = 0;i < width;i++) for(j = 0;j < height;j++)
	{
		unsigned char gray = (unsigned char)(0.5 + 255*cvGet2D(in, j, i).val[0]);
		setPixelProfile(&img, i, j, gray, gray, gray);
	}
	lockImage(&img);

	// Get frequency content, maybe from file.
	buildFrequencyContent(&img, &freqContent);

	// Halftone
	structureAwareErrorDiffusion(&img, &freqContent, &bitone);

	// Output result
	for(i = 0;i < width;i++) for(j = 0;j < height;j++)
		cvmSet(out, j, i, getRed(&bitone, i, j) / 255.0f);

	freeImage(&img);
	freeImage(&bitone);
	freeFrequencyContent(&freqContent);

	*_out = cv::Mat(out->rows,out->cols,CV_64FC1,out->data.fl).clone();
	_out->convertTo(*_out,CV_8UC1,255);
	for(i = 0;i<height;++i)
	{
		for(j = 0;j < width;++j)
		{
			_out->at<unsigned char>(i,j) = 255 - _out->at<unsigned char>(i,j);
		}
	}
}
