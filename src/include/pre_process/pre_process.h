#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core_c.h>
/**
 * @brief module strcut
 * data low 9 bit is the data 3*3 data matrix
 * 
 */
typedef struct
{
    int data;
    int center;
    int importance;
}module;
typedef struct
{
    int data;
    int center;
    double reliability;
}pattern;

double GetSimilirary(int data1,int data2);
double GetReliability(int data);