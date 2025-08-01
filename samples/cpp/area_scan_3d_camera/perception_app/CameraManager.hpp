#include <string>
#include <atomic>
#include <deque>
#include <memory>
#include <future>
#include <queue>
#include "area_scan_3d_camera/Camera.h"
#include "FrameSet.hpp"
#include "utils/CVWindow.hpp"

class CameraManager
{
public:
    CameraManager();
    ~CameraManager();

    bool Init();
    bool Start();
    bool Stop();

    bool Connect();

    void Capture(mmind::eye::Camera& camera, const std::string& suffix);

private:
    void CheckFilesLimit();

    void SaveImages(FrameSet& frame, const std::string& suffix);
    void SaveImagesAsync(FrameSet frame, const std::string suffix);  // 异步保存
    void CleanupCompletedSaves();  // 清理已完成的保存任务
    void WaitForAllSaves();  // 等待所有保存任务完成

    void ShowImages(FrameSet& frame);

    bool ProcessWindowEvents();

private:
    mmind::eye::Camera camera_;
    std::atomic<bool> is_running_ {false};
    std::deque<std::vector<std::string>> file_queue_;
    std::unique_ptr<CVWindow> display_window_;
    
    // 异步保存相关
    std::queue<std::future<void>> save_futures_;
    std::mutex save_mutex_;
    
    // 性能监控相关
    std::chrono::high_resolution_clock::time_point last_frame_time_;
    int frame_count_;
    std::chrono::high_resolution_clock::time_point performance_start_time_;
};