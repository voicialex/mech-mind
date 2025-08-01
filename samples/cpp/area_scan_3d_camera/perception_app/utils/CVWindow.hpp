#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <mutex>
#include "area_scan_3d_camera/Camera.h" // 添加Mech-Eye SDK头文件

class CVWindow {
public:
    // 创建窗口
    CVWindow(std::string name, uint32_t width = 1280, uint32_t height = 720);
    ~CVWindow();
    
    // 显示Frame2DAnd3D中的图像
    void showFrame2DAnd3D(const mmind::eye::Frame2DAnd3D& frame);
    
    // 处理窗口事件
    bool processEvents();
    
    // 关闭窗口
    void close();

private:
    // 转换Color2DImage为cv::Mat
    cv::Mat color2DImageToMat(const mmind::eye::Color2DImage& colorImage);
    
    // 转换DepthMap为伪彩色cv::Mat
    cv::Mat depthMapToColorMat(const mmind::eye::DepthMap& depthMap);
    
    std::string name_;
    uint32_t width_;
    uint32_t height_;
    bool closed_;
    cv::Mat renderMat_;
    std::mutex renderMutex_;
};
