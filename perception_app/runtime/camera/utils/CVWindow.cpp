#include "CVWindow.hpp"
#include "area_scan_3d_camera/api_util.h"  // Add Mech-Eye SDK header

#if defined(__has_include)
#if __has_include(<opencv2/core/Logger.hpp>)
#include <opencv2/core/Logger.hpp>
#define TO_DISABLE_OPENCV_LOG
#endif
#endif

const std::string defaultKeyMapPrompt = "'Esc': Exit Window, '?': Show Key Map";

CVWindow::CVWindow(std::string name, uint32_t width, uint32_t height)
    : name_(name), width_(width), height_(height), closed_(false) {
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

void CVWindow::showFrame2DAnd3D(const mmind::eye::Frame2DAnd3D &frame) {
  // Get 2D image
  mmind::eye::Color2DImage colorImage = frame.frame2D().getColorImage();
  cv::Mat colorMat = color2DImageToMat(colorImage);

  // Get depth map
  mmind::eye::DepthMap depthMap = frame.frame3D().getDepthMap();
  cv::Mat depthColorMat = depthMapToColorMat(depthMap);

  // Create combined image
  cv::Mat combined;
  if (!colorMat.empty() && !depthColorMat.empty()) {
    // Resize to maintain aspect ratio
    int targetHeight = 480;
    cv::Mat resizedColor, resizedDepth;
    cv::resize(colorMat, resizedColor, cv::Size(colorMat.cols * targetHeight / colorMat.rows, targetHeight));
    cv::resize(depthColorMat, resizedDepth,
               cv::Size(depthColorMat.cols * targetHeight / depthColorMat.rows, targetHeight));

    // Horizontal concatenation
    cv::hconcat(resizedColor, resizedDepth, combined);
  } else if (!colorMat.empty()) {
    combined = colorMat;
  } else if (!depthColorMat.empty()) {
    combined = depthColorMat;
  } else {
    return;
  }

  // Display image
  {
    std::lock_guard<std::mutex> lock(renderMutex_);
    renderMat_ = combined;
    cv::imshow(name_, combined);
  }
}

bool CVWindow::processEvents() {
  int key = cv::waitKey(1);
  if (key == 27) {  // ESC key
    closed_ = true;
  }
  return !closed_;
}

void CVWindow::close() { closed_ = true; }

cv::Mat CVWindow::color2DImageToMat(const mmind::eye::Color2DImage &colorImage) {
  if (colorImage.data() == nullptr) return cv::Mat();

  // Assume data is in RGB format
  cv::Mat rgbMat(colorImage.height(), colorImage.width(), CV_8UC3, (void *)colorImage.data());
  cv::Mat bgrMat;
  cv::cvtColor(rgbMat, bgrMat, cv::COLOR_RGB2BGR);
  return bgrMat;
}

cv::Mat CVWindow::depthMapToColorMat(const mmind::eye::DepthMap &depthMap) {
  if (depthMap.data() == nullptr) return cv::Mat();

  cv::Mat depthMat(depthMap.height(), depthMap.width(), CV_32FC1, (void *)depthMap.data());
  cv::Mat normalized, colorMap;

  // Normalize depth values
  double minVal, maxVal;
  cv::minMaxLoc(depthMat, &minVal, &maxVal);
  if (maxVal > minVal) {
    depthMat.convertTo(normalized, CV_8UC1, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
  } else {
    normalized = cv::Mat::zeros(depthMat.size(), CV_8UC1);
  }

  // Apply pseudo-color
  cv::applyColorMap(normalized, colorMap, cv::COLORMAP_JET);
  return colorMap;
}
