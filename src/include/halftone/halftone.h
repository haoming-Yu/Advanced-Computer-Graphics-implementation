#include"halftone/saed.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core_c.h>

// #include "opencv2/contrib/contrib.hpp"

int buildFrequencyContent(Image *img, FrequencyContent *out);
void cvHalftone(cv::Mat *in, cv::Mat* out);