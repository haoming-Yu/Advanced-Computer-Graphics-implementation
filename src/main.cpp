#include"halftone/halftone.h"
#include"pre_process/pre_process.h"
#include"optimize/optimize.h"
#include<iostream>
int main(void)
{
    cv::Mat halftone = cv::imread("../img/halftone.png",cv::ImreadModes::IMREAD_GRAYSCALE);
    cv::Mat qrcode = cv::imread("../img/qr.png",cv::ImreadModes::IMREAD_GRAYSCALE);
    cv::Mat importance = cv::imread("../img/border.png",cv::ImreadModes::IMREAD_GRAYSCALE);
    std::cout << halftone.rows << "," << halftone.cols << std::endl;
    std::cout << qrcode.rows << "," << qrcode.cols << std::endl;
    std::cout << importance.rows << "," << importance.cols << std::endl;
    std::vector<module> modules;
    std::vector<int> label;
    GetModules(halftone,qrcode,importance,modules);
    OptimizeBySwap(halftone.rows,halftone.cols,modules,label);
    // std::cout<<res.rows<<","<<res.cols<<std::endl;
    int index = 0;
    for(int i=0;i < qrcode.rows; i += 3)
    {
        for(int j=0; j < qrcode.cols; j += 3)
        {
            ConvertIdToPattern(qrcode,i,j,label[index++]);
        }
    }
    cv::imwrite("../img/res_qr.png",qrcode);
}