#include"halftone/halftone.h"
#include"pre_process/pre_process.h"
#include"qrcode/qrcode.h"
#include"optimize/optimize.h"
#include "ImpMap/ImpMap.h"
#include<iostream>
#include<vector>
#include<unordered_set>

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

std::string file_folder = "../video/";
std::string file_name = "video.mp4";

int main(void)
{
    // process video
    // 创建VideoCapture对象，用于读取视频文件
    cv::VideoCapture cap(file_folder + file_name);
    
    // 检查视频文件是否成功打开
    if (!cap.isOpened()) {
        std::cout << "无法打开视频文件！" << std::endl;
        return -1;
    }
    
    // 循环读取视频帧
    cv::Mat frame;
    int video_frames_count = 0;
    while (cap.read(frame)) {
        // 在这里可以对每一帧进行处理，比如显示、保存等
        cv::imwrite(file_folder + std::to_string(video_frames_count++) + ".jpg", frame);
    }
    
    // 关闭视频文件
    cap.release();
    

    // version
    int version = 5;
    int image_size = ((version-1)*4 + 21) * 3;
    // get qr code 
    cv::Mat qrcode;
    char qr_str[11] = "pt yyds!";
    std::unordered_set<int> necessary_idx;
    std::vector<mRSblock*> QR_rs_blocks;
    generate_qr(version, qr_str, necessary_idx, qrcode, QR_rs_blocks);
    std::cout << "QR rs blocks' number is: " << QR_rs_blocks.size() << std::endl; 
    for (int i = 0; i < QR_rs_blocks.size(); ++i) {
        //std::cout << i << "'s rs block code words size: " << QR_rs_blocks[i]->CodeWords.size() << std::endl;
        for (int j = 0; j < QR_rs_blocks[i]->CodeWords.size(); ++j) {
            CodeWord* cur_cw = QR_rs_blocks[i]->CodeWords[j];
            //std::cout << "\t" << "QR code words' modules' size: " << cur_cw->modules.size() << std::endl;
        }
    }
    //std::cout << "necessary modules size is: " << necessary_idx.size() << std::endl;
    cv::imwrite(file_folder + "qr.png", qrcode);

    for (int i = 0; i < video_frames_count; i++){
        // split to channels
        cv::Mat original = cv::imread(file_folder + std::to_string(i) + ".jpg", cv::IMREAD_COLOR);
        std::cout << "channels:" << original.channels() << std::endl;
        std::cout << "type:" << original.type() << std::endl;

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
        cv::imwrite(file_folder + "res/" + "res" + std::to_string(i) + ".png", result);
        // std::cout << "input" << std::endl;
    }


    // 生成视频
    std::string output_file = file_folder + "res/output_video.avi";
    int frame_width, frame_height;
    cv::VideoWriter video;

    // 获取第一帧图像的大小
    cv::Mat first_frame = cv::imread(file_folder + "res/" + "res0.png");
    if (first_frame.empty()) {
        std::cout << "无法读取第一帧图像！" << std::endl;
        return -1;
    }
    frame_width = image_size;
    frame_height = image_size;

    // 创建VideoWriter对象，用于写入视频文件
    video.open(output_file, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, cv::Size(frame_width, frame_height));

    if (!video.isOpened()) {
        std::cout << "无法创建视频文件！" << std::endl;
        return -1;
    }

    // 循环读取图像帧并写入视频文件
    int i = 0;
    while (i < video_frames_count) {
        std::string frame_file = file_folder + "res/" + "res" + std::to_string(i) + ".png";
        cv::Mat frame = cv::imread(frame_file);
        if (frame.empty()) {
            std::cout << "无法读取图像帧！" << std::endl;
            break;
        }

        // 将当前帧写入视频文件
        video.write(frame);

        i++;
    }

    // 释放VideoWriter对象并关闭视频文件
    video.release();

    std::cout << "视频文件已生成：" << output_file << std::endl;


    return 0;
    

}

