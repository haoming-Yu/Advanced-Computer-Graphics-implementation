#ifndef _PRE_PROCESS_
#define _PRE_PROCESS_
#include <vector>
#include <assert.h>
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

/*
 * In order to avoid conflicts
 * I create this module as a test for 
 * replicating the code from QR part
 * 
 */
class QRmodule
{
public:
    QRmodule() {
        init();
    }
    ~QRmodule() {
        init();
    }

    void init(void) {
        type = -1;
        pos = cv::Point(0, 0);
        QRC = -1;
        RSID = -1;
        CWID = -1;
        PID1 = -1;
        PID2 = -1;
        weit = -1.0;
    }

public:
    // 0:Codewords
    // 1:Remaining Bits
    // 2:Version Information
    // 3:Format Information 
    // 4:Alignment Patterns
    // 5:Timing Patterns
    // 6:Separators and Position Detection Patterns
    int type;
    cv::Point pos;
    int QRC; // qr code value (1-black, 0-white)
    int RSID; // RS block ID (1-based index, -1 indicates non data module)
    int CWID; // Codeword ID in the RS block (1-based index, -1 indicates non data module)
    int PID1; // Ideal/Original pattern ID
    int PID2; // optimal pattern ID
    // int VID; // vertex index in the graph -> NO use for us, we use matrix instead of graph
    double weit; // importance of the module (0-1, -1 indicates non data module)
};

typedef struct
{
    int data;
    int center;
    double reliability;
}pattern;

double GetSimilirary(int data1,int data2);
double GetReliability(int data);
int GetPatternId(const cv::Mat &mat,int row_location,int col_location);
void ConvertIdToPattern(cv::Mat &mat,int row_location,int col_location, int id);
void GetModules(const cv::Mat& halftone_img,const cv::Mat& qrcode_img,const cv::Mat& importance_map,std::vector<module> &modules);
// double GetEdgeWeight(int data1,int data2);
#endif