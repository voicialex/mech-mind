#include "CVWindow.hpp"
#include "area_scan_3d_camera/api_util.h" // 添加Mech-Eye SDK头文件

#if defined(__has_include)
# if __has_include(<opencv2/core/utils/logger.hpp>)
#  include <opencv2/core/utils/logger.hpp>
#  define TO_DISABLE_OPENCV_LOG
# endif
#endif

const std::string defaultKeyMapPrompt = "'Esc': Exit Window, '?': Show Key Map";
CVWindow::CVWindow(std::string name, uint32_t width, uint32_t height)
    : name_(name),
      width_(width),
      height_(height),
      closed_(false) {

#if defined(TO_DISABLE_OPENCV_LOG)
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
#endif

    cv::namedWindow(name_, cv::WINDOW_NORMAL);
    cv::resizeWindow(name_, width_, height_);
}

CVWindow::~CVWindow() noexcept {
    close();
    cv::destroyWindow(name_);
}

void CVWindow::showFrame2DAnd3D(const mmind::eye::Frame2DAnd3D& frame) {
    // 获取2D图像
    mmind::eye::Color2DImage colorImage = frame.frame2D().getColorImage();
    cv::Mat colorMat = color2DImageToMat(colorImage);
    
    // 获取深度图
    mmind::eye::DepthMap depthMap = frame.frame3D().getDepthMap();
    cv::Mat depthColorMat = depthMapToColorMat(depthMap);
    
    // 创建组合图像
    cv::Mat combined;
    if (!colorMat.empty() && !depthColorMat.empty()) {
        // 调整大小保持宽高比
        int targetHeight = 480;
        cv::Mat resizedColor, resizedDepth;
        cv::resize(colorMat, resizedColor, cv::Size(colorMat.cols * targetHeight / colorMat.rows, targetHeight));
        cv::resize(depthColorMat, resizedDepth, cv::Size(depthColorMat.cols * targetHeight / depthColorMat.rows, targetHeight));
        
        // 水平拼接
        cv::hconcat(resizedColor, resizedDepth, combined);
    } else if (!colorMat.empty()) {
        combined = colorMat;
    } else if (!depthColorMat.empty()) {
        combined = depthColorMat;
    } else {
        return;
    }
    
    // 显示图像
    {
        std::lock_guard<std::mutex> lock(renderMutex_);
        renderMat_ = combined;
        cv::imshow(name_, combined);
    }
}

bool CVWindow::processEvents() {
    int key = cv::waitKey(1);
    if (key == 27) { // ESC键
        closed_ = true;
    }
    return !closed_;
}

void CVWindow::close() {
    closed_ = true;
}

cv::Mat CVWindow::color2DImageToMat(const mmind::eye::Color2DImage& colorImage) {
    if (colorImage.data() == nullptr) return cv::Mat();
    
    // 假设数据是RGB格式
    cv::Mat rgbMat(colorImage.height(), colorImage.width(), CV_8UC3, (void*)colorImage.data());
    cv::Mat bgrMat;
    cv::cvtColor(rgbMat, bgrMat, cv::COLOR_RGB2BGR);
    return bgrMat;
}

cv::Mat CVWindow::depthMapToColorMat(const mmind::eye::DepthMap& depthMap) {
    if (depthMap.data() == nullptr) return cv::Mat();
    
    cv::Mat depthMat(depthMap.height(), depthMap.width(), CV_32FC1, (void*)depthMap.data());
    cv::Mat normalized, colorMap;
    
    // 归一化深度值
    double minVal, maxVal;
    cv::minMaxLoc(depthMat, &minVal, &maxVal);
    if (maxVal > minVal) {
        depthMat.convertTo(normalized, CV_8UC1, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
    } else {
        normalized = cv::Mat::zeros(depthMat.size(), CV_8UC1);
    }
    
    // 应用伪彩色
    cv::applyColorMap(normalized, colorMap, cv::COLORMAP_JET);
    return colorMap;
}
