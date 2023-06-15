#include"halftone/halftone.h"
#include"pre_process/pre_process.h"
#include"qrcode/qrcode.h"
#include"optimize/optimize.h"
#include<iostream>
#include<unordered_set>

int main(void)
{
    cv::Mat halftone = cv::imread("../img/37x37/halftone.png",cv::ImreadModes::IMREAD_GRAYSCALE);
    // cv::Mat qrcode = cv::imread("../img/37x37/qr.png",cv::ImreadModes::IMREAD_GRAYSCALE);
    cv::Mat importance = cv::imread("../img/37x37/border.png",cv::ImreadModes::IMREAD_GRAYSCALE);
    cv::Mat qrcode;
    int version = 5;
    char qr_str[11] = "YuSir YYDS";
    std::unordered_set<int> necessary_idx;
    std::cout << "OK0" << std::endl;
    generate_qr(version, qr_str, necessary_idx, qrcode); 
    std::cout << "necessary modules size is: " << necessary_idx.size() << std::endl;
    cv::imwrite("../img/37x37/qr.png", qrcode);

    std::cout << "input" << std::endl;
    std::cout << halftone.rows << "," << halftone.cols << std::endl;
    std::cout << qrcode.rows << "," << qrcode.cols << std::endl;
    std::cout << importance.rows << "," << importance.cols << std::endl;

    std::vector<module> modules;
    std::vector<int> label;
    GetModules(halftone,qrcode,importance,modules);
    OptimizeBySwap(halftone.rows,halftone.cols,modules,label);

    int index = 0;
    int error_module = 0; // 指的是那些中心颜色和原二维码不一致的module
    for(int i=0;i < qrcode.rows; i += 3)
    {
        for(int j=0; j < qrcode.cols; j += 3)
        {
            int copy = qrcode.at<uchar>(i + 1, j + 1);
            if ( necessary_idx.find((i/3)*(qrcode.cols/3) + j/3) != necessary_idx.end() ){
                index++;
            }
            else{
                ConvertIdToPattern(qrcode,i,j,label[index++]);
            }

            if (qrcode.at<uchar>(i + 1, j + 1) != copy)
                error_module++;
                qrcode.at<uchar>(i + 1, j + 1) = copy;
        }
    }
    std::cout << "error modules: " << error_module << std::endl;
    cv::imwrite("../img/37x37/res_qr.png",qrcode);
}