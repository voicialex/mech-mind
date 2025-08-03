#include <string>
#include <atomic>
#include <deque>
#include <memory>
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

    void ShowImages(FrameSet& frame);

private:
    mmind::eye::Camera camera_;
    std::atomic<bool> is_running_ {false};
    std::deque<std::vector<std::string>> file_queue_;
    std::unique_ptr<CVWindow> display_window_;
};