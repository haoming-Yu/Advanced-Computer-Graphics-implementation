#include"halftone/halftone.h"
#include"pre_process/pre_process.h"
#include"qrcode/qrcode.h"
#include"optimize/optimize.h"
#include<iostream>
#include<vector>
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
    std::vector<mRSblock*> QR_rs_blocks;
    generate_qr(version, qr_str, necessary_idx, qrcode, QR_rs_blocks);
    std::cout << "QR rs blocks' number is: " << QR_rs_blocks.size() << std::endl; 
    for (int i = 0; i < QR_rs_blocks.size(); ++i) {
        std::cout << i << "'s rs block code words size: " << QR_rs_blocks[i]->CodeWords.size() << std::endl;
        for (int j = 0; j < QR_rs_blocks[i]->CodeWords.size(); ++j) {
            CodeWord* cur_cw = QR_rs_blocks[i]->CodeWords[j];
            std::cout << "\t" << "QR code words' modules' size: " << cur_cw->modules.size() << std::endl;
        }
    }
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
    for(int i=0;i < qrcode.rows; i += 3)
    {
        for(int j=0; j < qrcode.cols; j += 3)
        {
            if ( necessary_idx.find((i/3)*(qrcode.cols/3) + j/3) != necessary_idx.end() ){
                std::cout << (i/3)*(qrcode.cols/3) + j/3 << " ";
                index++;
            }
            else{
                ConvertIdToPattern(qrcode,i,j,label[index++]);
            }
        }
    }
    cv::imwrite("../img/37x37/res_qr.png",qrcode);
    std::cout << std::endl;
}