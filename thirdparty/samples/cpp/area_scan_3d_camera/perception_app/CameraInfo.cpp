#include "CameraInfo.hpp"
#include "area_scan_3d_camera/PointCloudTransformation.h"

void CameraInfo::InitCameraInfo(mmind::eye::Camera& camera)
{
    camera_ = camera;
    transformation_ = getTransformationParams(camera);
    camera.getCameraIntrinsics(cameraIntrinsics_);
}
