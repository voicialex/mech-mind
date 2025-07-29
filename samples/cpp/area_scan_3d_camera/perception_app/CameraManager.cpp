
#include <chrono>
#include <thread>
#include "CameraManager.hpp"
#include <opencv2/imgcodecs.hpp>
#include "area_scan_3d_camera/api_util.h"
#include "ConfigHelper.hpp"

CameraManager::CameraManager()
{
}

CameraManager::~CameraManager()
{
}

bool CameraManager::Init()
{
    return true;
}

bool CameraManager::Connect()
{
    if (!findAndConnect(camera_))
        return false;

    mmind::eye::CameraInfo cameraInfo;
    showError(camera_.getCameraInfo(cameraInfo));
    printCameraInfo(cameraInfo);

    if (!confirmCapture3D()) {
        camera_.disconnect();
        return false;
    }

    is_running_ = true;
    return true;
}

bool CameraManager::Start()
{
    if (!is_running_)
        return false;

    while (is_running_)
    {
        std::string suffix = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        capture(camera_, suffix);
    }

    return true;
}

bool CameraManager::Stop()
{
    is_running_ = false;
    camera_.disconnect();
    return true;
}

void CameraManager::capture(mmind::eye::Camera& camera, const std::string& suffix)
{
    if (!is_running_)
        return;

    mmind::eye::Frame2DAnd3D frame2DAnd3D;
    showError(camera.capture2DAnd3D(frame2DAnd3D));

    // 2D image
    if (ConfigHelper::getInstance().save_config_.save_2d_image)
    {
        std::string image_file = ConfigHelper::getInstance().save_config_.save_2d_image_file(suffix);
        mmind::eye::Color2DImage colorImage = frame2DAnd3D.frame2D().getColorImage();
        cv::Mat color8UC3 = cv::Mat(colorImage.height(), colorImage.width(), CV_8UC3, colorImage.data());
        cv::imwrite(image_file, color8UC3);
        std::cout << "Capture and save the 2D image: " << image_file << std::endl;
    }

    // Depth map
    if (ConfigHelper::getInstance().save_config_.save_depth_map)
    {
        std::string depth_file = ConfigHelper::getInstance().save_config_.save_depth_map_file(suffix);
        mmind::eye::DepthMap depthMap = frame2DAnd3D.frame3D().getDepthMap();
        cv::Mat depth32F = cv::Mat(depthMap.height(), depthMap.width(), CV_32FC1, depthMap.data());
        cv::imwrite(depth_file, depth32F);
        std::cout << "Capture and save the depth map: " << depth_file << std::endl;
    }

    // Point cloud
    if (ConfigHelper::getInstance().save_config_.save_point_cloud)
    {
        std::string pointCloudFile = ConfigHelper::getInstance().save_config_.save_point_cloud_file(suffix);
        showError(frame2DAnd3D.frame3D().saveUntexturedPointCloud(mmind::eye::FileFormat::PLY, pointCloudFile),
                  "Capture and save the untextured point cloud: " + pointCloudFile);
    }
    if (ConfigHelper::getInstance().save_config_.save_textured_point_cloud)
    {
        std::string texturedPointCloudFile = ConfigHelper::getInstance().save_config_.save_textured_point_cloud_file(suffix);
        showError(frame2DAnd3D.saveTexturedPointCloud(mmind::eye::FileFormat::PLY, texturedPointCloudFile),
                  "Capture and save the textured point cloud: " + texturedPointCloudFile);
    }
}   
