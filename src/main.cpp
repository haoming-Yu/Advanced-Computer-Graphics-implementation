#include"halftone/halftone.h"
#include"pre_process/pre_process.h"
#include"qrcode/qrcode.h"
#include"optimize/optimize.h"
#include "ImpMap/ImpMap.h"
#include<iostream>
#include<vector>
#include<unordered_set>

std::string file_folder = "../img/zx/";
std::string file_name = "zx.jpg";

void ImageChannelProcess(cv::Mat& HalftoneMat,cv::Mat& QrMat,cv::Mat& ImpMat,cv::Mat& ResMat,std::unordered_set<int>& necessary_idx)
{
    // preprocess
    std::vector<module> modules;
    std::vector<int> label;
    GetModules(HalftoneMat,QrMat,ImpMat,modules);

    // swap optimization
    OptimizeBySwap(QrMat.rows,QrMat.cols,modules,label);

    // fix error module
    int index = 0;
    int error_module = 0; // 指的是那些中心颜色和原二维码不一致的module
    ResMat = HalftoneMat;
    for(int i=0;i < QrMat.rows; i += 3)
    {
        for(int j=0; j < QrMat.cols; j += 3)
        {
            int copy = QrMat.at<uchar>(i + 1, j + 1);
            if ( necessary_idx.find((i/3)*(QrMat.cols/3) + j/3) != necessary_idx.end() ){
                index++;
                for(int m=0;m<3;++m)
                {
                    for(int n=0;n<3;++n)
                    {
                        ResMat.at<uchar>(i+m,j+n) = QrMat.at<uchar>(i+m,j+n);
                    }
                }
            }
            else{
                ConvertIdToPattern(ResMat,i,j,label[index++]);
            }

            if (QrMat.at<uchar>(i + 1, j + 1) != copy)
                error_module++;
                ResMat.at<uchar>(i + 1, j + 1) = copy;
        }
    }
    std::cout << "error modules: " << error_module << std::endl;
}
int main(void)
{
    // version
    int version = 5;
    int image_size = ((version-1)*4 + 21) * 3;
    // split to channels
    cv::Mat original = cv::imread(file_folder + file_name,-1);
    
    // resize
    cv::resize(original,original,cv::Size(image_size,image_size),0.0,0.0,cv::INTER_CUBIC);
    // get channel counts
    int channel_counts = original.channels();
    cv::Mat* channels = new cv::Mat[channel_counts];
    // split
    cv::split(original,channels);
    // test merge
    // cv::Mat test_merge;
    // cv::merge(channels,channel_counts,test_merge);
    // cv::imwrite(file_folder+"test_merge.png",test_merge);
    for(int i=0;i<channel_counts;++i)
    {
        cv::imwrite(file_folder+"channels_origin_"+std::to_string(i)+".png",channels[i]);
    }

    // get importance map
    cv::Mat importance;
    GetBorderImg(original, importance);
    cv::imwrite(file_folder + "importance.png",importance);

    // get qr code 
    cv::Mat qrcode;
    char qr_str[11] = "YuuuuSir!";
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
    cv::imwrite(file_folder + "qr.png", qrcode);

    // handle each channel
    for(int i = 0; i < channel_counts; ++i)
    {
        // get channels halftone img
        cv::Mat halftone;
        cv::resize(channels[i],channels[i],cv::Size(image_size,image_size),0.0,0.0,cv::INTER_CUBIC);
        cvHalftone(channels+i,channels+i);
        cv::imwrite(file_folder + "halftone_channel_"+std::to_string(i)+".png",channels[i]);
        ImageChannelProcess(channels[i],qrcode,importance,channels[i],necessary_idx);
        cv::imwrite(file_folder + "res_channel_"+std::to_string(i)+".png",channels[i]);
    } 
    // merge channels 
    cv::Mat result;
    cv::merge(channels,channel_counts,result);
    // save
    cv::imwrite(file_folder+"res_qr.png",result);
    // std::cout << "input" << std::endl;

}