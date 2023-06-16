// #include <opencv.hpp>
#include "ImpMap/ImpMap.h"

// teste
typedef Matx<float, 2, 2> tensor;
// bilateral variables
float sst = 2;
int gaussian_size = ceil(sst * 2) * 2 + 1;
float sigma_d = 3.0f;
float sigma_r = 4.25f;
float sig_e = 1.0;
float sig_r = 1.6;
float sig_m = 3.0;
float tau = 0.99;
const double PI = 3.1415926f;

static void GetGaussianWeights(float *weights,
							   int neighbor,
							   float sigma)
{
	if ((NULL == weights) || (neighbor < 0))
		return;
	float term1 = 1.0 / (sqrt(2.0 * PI) * sigma);
	float term2 = -1.0 / (2 * pow(sigma, 2));
	weights[neighbor] = term1;
	float sum = weights[neighbor];
	for (int i = 1; i <= neighbor; ++i)
	{
		weights[neighbor + i] = exp(pow(i, 2) * term2) * term1;
		weights[neighbor - i] = weights[neighbor + i];
		sum += weights[neighbor + i] + weights[neighbor - i];
	}
	// Normalization
	for (int j = 0; j < neighbor * 2 + 1; ++j)
	{
		weights[j] /= sum;
	}
}

// Prepare 1-d difference of gaussian template.
static void GetDiffGaussianWeights(float *weights,
								   int neighbor,
								   float sigma_e,
								   float sigma_r,
								   float tau)
{
	if ((NULL == weights) || (neighbor < 0))
		return;
	float *gaussian_e = new float[neighbor * 2 + 1];
	float *gaussian_r = new float[neighbor * 2 + 1];
	GetGaussianWeights(gaussian_e, neighbor, sigma_e);
	GetGaussianWeights(gaussian_r, neighbor, sigma_r);
	float sum = 0;
	for (int i = 0; i < neighbor * 2 + 1; ++i)
	{
		weights[i] = gaussian_e[i] - tau * gaussian_r[i];
		sum += weights[i];
	}
	// Normalization
	for (int j = 0; j < neighbor * 2 + 1; ++j)
	{
		weights[j] /= sum;
	}
	delete[] gaussian_e;
	delete[] gaussian_r;
}

Mat color_quantize(Mat src, int s)
{
	float step = 1.0 / s;
	vector<float> shades(s);

	for (int i = 0; i < s; i++)
	{
		shades[i] = step * (i + 0.5);
	}

	Mat m(src.size(), CV_32F);
	for (int r = 0; r < src.rows; r++)
	{
		float *_s = src.ptr<float>(r);
		float *_m = m.ptr<float>(r);
		for (int c = 0; c < src.cols; c++)
		{
			int n = floor(_s[c] / step);

			if (n == s) // evito sair do limite de tons
				n -= 1;

			_m[c] = shades[n];
		}
	}
	return m;
}
void GetBorderImg(cv::Mat& in,cv::Mat& out)
{
    Mat im = in;
	Mat gray, lab;
	int rows = im.rows;
	int cols = im.cols;

	cvtColor(im, gray, COLOR_BGR2GRAY);
	
	Mat smooth;
	bilateralFilter(gray, smooth, 6, 150, 150);

	Mat dx, dy;
	Sobel(smooth, dx, CV_32F, 1, 0);
	Sobel(smooth, dy, CV_32F, 0, 1);

	Mat jacob; //= Mat::zeros(rows, cols, CV_32FC3);;
	jacob.create(rows, cols, CV_32FC3);

	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			float gx = dx.at<float>(i, j);
			float gy = dy.at<float>(i, j);

			jacob.at<Vec3f>(i, j)[0] = gx * gx;
			jacob.at<Vec3f>(i, j)[1] = gy * gy;
			jacob.at<Vec3f>(i, j)[2] = gx * gy;
		}
	}

	GaussianBlur(jacob, jacob, Size2i(gaussian_size, gaussian_size), sst);

	// ETF
	Mat ETF = Mat::zeros(rows, cols, CV_32FC3);
	float E, G, F, lambda, v2x, v2y, v2;

	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			E = jacob.at<Vec3f>(i, j)[0];
			G = jacob.at<Vec3f>(i, j)[1];
			F = jacob.at<Vec3f>(i, j)[2];

			lambda = 0.5 * (E + G + sqrtf((G - E) * (G - E) + 4 * F * F));
			v2x = E - lambda;
			v2y = F;
			v2 = sqrtf(v2x * v2x + v2y * v2y);

			ETF.at<Vec3f>(i, j)[0] = (0 == v2) ? 0 : (v2x / v2);

			// ETF.at<Vec3f>(i, j)[0] = (0 == v2) ? 0 : (v2x);

			ETF.at<Vec3f>(i, j)[1] = (0 == v2) ? 0 : (v2y / v2);

			// ETF.at<Vec3f>(i, j)[1] = (0 == v2) ? 0 : (v2y);

			ETF.at<Vec3f>(i, j)[2] = sqrtf(E + G - lambda);
		}
	}

	// BILATERAL FILTER + EDGE EXTRACTION
	Mat FDOG;
	FDOG.create(rows, cols, CV_32FC1);
	// FDOG = 0;

	Mat f0 = Mat::ones(rows, cols, CV_32FC1);
	Mat f1 = Mat::ones(rows, cols, CV_32FC1);

	Mat u1 = Mat::zeros(im.size(), CV_8UC1);

	int near = (int)(ceilf(2 * sig_r));
	float sin, cos;

	float *gauss_w = new float[near * 2 + 1];

	float *sample1, *sample2;

	float sum_diff, sum_dev, sum_1;

	GetDiffGaussianWeights(gauss_w, near, sig_e, sig_r, tau);

	int near2 = ceilf(2 * sig_m);

	float *gauss_w2 = new float[near2 * 2 + 1];

	GetGaussianWeights(gauss_w2, near2, sig_m);
	Mat gr_scale;
	smooth.copyTo(gr_scale);

	// gradient
	for (int i = near; i < (rows - near); ++i)
	{
		for (int j = near; j < (cols - near); ++j)
		{
			cos = ETF.at<Vec3f>(i, j)[1];
			sin = -1 * ETF.at<Vec3f>(i, j)[0];
			sample1 = new float[near * 2 + 1];
			sample1[near] = static_cast<float>(gr_scale.at<uchar>(i, j));
			for (int k = 1; k <= near; ++k)
			{
				int r = round(sin * k);
				int c = round(cos * k);

				sample1[near + k] = static_cast<float>(gr_scale.at<uchar>(i + r, j + c));
				sample1[near - k] = static_cast<float>(gr_scale.at<uchar>(i - r, j - c));
			}

			sum_diff = 0;
			sum_dev = 0;

			for (int k = 0; k < 2 * near + 1; ++k)
			{
				sum_diff += sample1[k] * gauss_w[k];
			}
			f0.at<float>(i, j) = sum_diff;
			delete[] sample1;
		}
	}

	// tangent
	for (int i = near2; i < (rows - near2); ++i)
	{
		for (int j = near2; j < (cols - near2); ++j)
		{
			cos = ETF.at<Vec3f>(i, j)[0];
			sin = ETF.at<Vec3f>(i, j)[1];
			sample2 = new float[near2 * 2 + 1];
			sample2[near2] = f0.at<float>(i, j);

			for (int k = 1; k <= near2; ++k)
			{
				int r = round(sin * k);
				int c = round(cos * k);

				sample2[near2 + k] = f0.at<float>(i + r, j + c);
				sample2[near2 - k] = f0.at<float>(i - r, j - c);
			}

			sum_1 = 0;

			for (int k = 0; k < 2 * near2 + 1; ++k)
			{
				sum_1 += sample2[k] * gauss_w2[k];
			}

			f1.at<float>(i, j) = sum_1;
			if (f1.at<float>(i, j) > 0)
			{
				// u1.at<uchar>(i, j) = 0;
				FDOG.at<float>(i, j) = 255;
			}
			else
			{
				// u1.at<uchar>(i, j) = 255;
				FDOG.at<float>(i, j) = 0;
			}
			delete[] sample2;
		}
	}

	delete[] gauss_w;
	delete[] gauss_w2;

	// QUANTIZATION

	Mat lum;
	vector<Mat> lab_channels;

	cvtColor(im, lab, COLOR_BGR2Lab);

	Mat lab1;
	bilateralFilter(lab, lab1, 6, 150, 150);
	Mat lab2;
	bilateralFilter(lab1, lab2, 6, 150, 150);
	Mat lab3;
	bilateralFilter(lab2, lab3, 6, 150, 150);

	split(lab3, lab_channels);
	lab_channels[0].convertTo(lum, CV_32F, 1.0f / 255.0f);

	Mat qt = color_quantize(lum, 8);

	qt.convertTo(qt, CV_8U, 255);

	lab_channels[0] = qt;

	Mat qtzd;
	merge(lab_channels, qtzd);

	cvtColor(qtzd, qtzd, COLOR_Lab2BGR);

	FDOG.convertTo(FDOG, CV_8U, 255);
	cvtColor(FDOG, FDOG, COLOR_GRAY2BGR);
    out = FDOG;
}
