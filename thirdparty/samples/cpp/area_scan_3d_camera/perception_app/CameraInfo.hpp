#pragma once

#include "area_scan_3d_camera/Camera.h"

class CameraInfo
{
public:
    static CameraInfo &getInstance()
    {
        static CameraInfo instance;
        return instance;
    }

    void InitCameraInfo(mmind::eye::Camera& camera);

public:
    mmind::eye::Camera camera_;
    mmind::eye::FrameTransformation transformation_;
    mmind::eye::CameraIntrinsics cameraIntrinsics_;

private:
    CameraInfo() = default;
    ~CameraInfo() = default;
    CameraInfo(const CameraInfo &) = delete;
    CameraInfo &operator=(const CameraInfo &) = delete;
};
