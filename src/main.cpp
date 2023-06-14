#include"halftone/halftone.h"
#include"pre_process/pre_process.h"
#include"qrcode/qrcode.h"
#include"optimize/optimize.h"
#include<iostream>
#include<unordered_set>

int main(void)
{
    cv::Mat halftone = cv::imread("../img/37x37/halftone.png",cv::ImreadModes::IMREAD_GRAYSCALE);
    cv::Mat qrcode = cv::imread("../img/37x37/qr.png",cv::ImreadModes::IMREAD_GRAYSCALE);
    cv::Mat importance = cv::imread("../img/37x37/border.png",cv::ImreadModes::IMREAD_GRAYSCALE);
    cv::Mat qr_code_test;
    int version = 5;
    char qr_str[6] = "Hello";
    std::unordered_set<int> necessary_idx;
    std::cout << "OK0" << std::endl;
    generate_qr(version, qr_str, necessary_idx, qr_code_test); 
    std::cout << "Size of necessary modules are: " << necessary_idx.size() << std::endl;
    cv::imwrite("../img/generated_qr.png", qr_code_test);

    std::cout << "input" << std::endl;
    std::cout << halftone.rows << "," << halftone.cols << std::endl;
    std::cout << qrcode.rows << "," << qrcode.cols << std::endl;
    std::cout << importance.rows << "," << importance.cols << std::endl;

    std::vector<module> modules;
    std::vector<int> label;
    GetModules(halftone,qrcode,importance,modules);
    OptimizeBySwap(halftone.rows,halftone.cols,modules,label);

    // std::cout<<res.rows<<","<<res.cols<<std::endl;

    int corner_size = 26;
    int small_orint_block_origin = 84;
    int small_orint_block_size = 15;
    int index = 0;
    for(int i=0;i < qrcode.rows; i += 3)
    {
        for(int j=0; j < qrcode.cols; j += 3)
        {
            if ( i == 18 || j == 18
                ||i < corner_size && j < corner_size
                || i < corner_size && j >= qrcode.cols - 1 - corner_size
                || i >= qrcode.rows - 1 - corner_size && j < corner_size
                || i >= small_orint_block_origin && i < small_orint_block_origin + small_orint_block_size && 
                    j >= small_orint_block_origin && j < small_orint_block_origin + small_orint_block_size){
               index++;
            }
            else{
                ConvertIdToPattern(qrcode,i,j,label[index++]);
            }
        }
    }
    cv::imwrite("../img/37x37/res_qr.png",qrcode);
}