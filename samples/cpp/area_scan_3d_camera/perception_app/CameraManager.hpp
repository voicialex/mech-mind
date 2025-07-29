#include <string>
#include <atomic>
#include "area_scan_3d_camera/Camera.h"

class CameraManager
{
public:
    CameraManager();
    ~CameraManager();

    bool Init();
    bool Start();
    bool Stop();

    bool Connect();

    void capture(mmind::eye::Camera& camera, const std::string& suffix);

private:
    mmind::eye::Camera camera_;
    std::atomic<bool> is_running_ {false};
};