// Copyright (c) Orbbec Inc. All Rights Reserved.
// Licensed under the MIT License.

#pragma once
#include <libobsensor/ObSensor.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cmath>
#include <map>
#include <atomic>
#include <chrono>

#include "utils.hpp"

namespace ob_smpl {

// arrange type
typedef enum {
    ARRANGE_SINGLE,      // Only show the first frame
    ARRANGE_ONE_ROW,     // Arrange the frames in the array as a row
    ARRANGE_ONE_COLUMN,  // Arrange the frames in the array as a column
    ARRANGE_GRID,        // Arrange the frames in the array as a grid
    ARRANGE_OVERLAY      // Overlay the first two frames in the array
} ArrangeMode;

class CVWindow {
public:
    // create a window with the specified name, width and height
    CVWindow(std::string name, uint32_t width = 1280, uint32_t height = 720, ArrangeMode arrangeMode = ARRANGE_SINGLE);
    ~CVWindow() noexcept;

    // 分离窗口显示逻辑和处理逻辑
    // 处理窗口消息和键盘输入，但不显示图像，返回窗口是否应该继续运行
    bool processEvents();

    // 显示当前渲染图像 - 必须在主线程中调用
    void updateWindow();

    // 获取当前渲染图像的副本
    cv::Mat getRenderMat();

    // close window
    void close();

    // clear cached frames and mats
    void reset();

    // add frames to view
    void pushFramesToView(std::vector<std::shared_ptr<const ob::Frame>> frames, int groupId = 0);
    void pushFramesToView(std::shared_ptr<const ob::Frame> currentFrame, int groupId = 0);

    // set show frame info
    void setShowInfo(bool show);

    // set show frame syncTime info
    void setShowSyncTimeInfo(bool show);

    // set alpha, only valid when arrangeMode_ is ARRANGE_OVERLAY
    void setAlpha(float alpha);

    // set the window size
    void resize(int width, int height);

    // set the key pressed callback
    void setKeyPressedCallback(std::function<void(int)> callback);

    // set the key prompt
    void setKeyPrompt(const std::string &prompt);

    // set the log message
    void addLog(const std::string &log);

    // destroyWindow
    void destroyWindow();

    // 无信号画面管理方法
    void showNoSignalFrame();
    void hideNoSignalFrame();
    bool isShowingNoSignalFrame() const;
    void updateNoSignalFrame(); // 更新无信号画面（如时间戳）

private:
    // frames processing thread function
    void processFrames();

    // arrange frames in the renderMat_ according to the arrangeMode_
    void arrangeFrames();

    // add info to mat
    cv::Mat visualize(std::shared_ptr<const ob::Frame> frame);

    // draw info to mat
    void drawInfo(cv::Mat &imageMat, std::shared_ptr<const ob::VideoFrame> &frame);

    cv::Mat resizeMatKeepAspectRatio(const cv::Mat &mat, int width, int height);

    // 无信号画面相关方法
    void createNoSignalFrame();
    
    // 更新无信号画面时间戳（内部方法）
    void checkAndUpdateNoSignalFrame();

private:
    std::string name_;
    ArrangeMode arrangeMode_;
    uint32_t    width_;
    uint32_t    height_;
    bool        closed_;
    bool        showInfo_;
    bool        showSyncTimeInfo_;
    bool        isWindowDestroyed_;
    float       alpha_;

    std::thread                                                  processThread_;
    std::map<int, std::vector<std::shared_ptr<const ob::Frame>>> srcFrameGroups_;
    std::mutex                                                   srcFrameGroupsMtx_;
    std::condition_variable                                      srcFrameGroupsCv_;

    using StreamsMatMap = std::map<int, std::pair<std::shared_ptr<const ob::Frame>, cv::Mat>>;
    StreamsMatMap matGroups_;
    std::mutex    renderMatsMtx_;
    cv::Mat       renderMat_;

    std::string prompt_;
    bool        showPrompt_;
    uint64      winCreatedTime_;

    std::string log_;
    uint64      logCreatedTime_;

    std::function<void(int)> keyPressedCallback_;

    // 无信号画面相关成员变量
    std::atomic<bool> showingNoSignalFrame_;
    cv::Mat noSignalMat_;
    std::chrono::steady_clock::time_point lastNoSignalUpdateTime_;
    std::mutex noSignalMutex_;
};

}  // namespace ob_smpl
