#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <string>
#include <math.h>
#include <vector>
using namespace cv;
using namespace std;

void GetBorderImg(cv::Mat& in,cv::Mat& out);