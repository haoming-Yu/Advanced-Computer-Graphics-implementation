#include"halftone/halftone.h"
#include<iostream>
int main(void)
{
    cv::Mat in = cv::imread("../img/beihaiting.jfif",cv::ImreadModes::IMREAD_COLOR);
    cv::Mat out(in.rows,in.cols,CV_64FC1),*_out = &out;

    cvHalftone(&in,&out);
    // std::cout<<res.rows<<","<<res.cols<<std::endl;

    cv::imwrite("../img/halftone1.png",out);
}