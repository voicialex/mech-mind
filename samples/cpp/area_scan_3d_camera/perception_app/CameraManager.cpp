
#include <chrono>
#include <thread>
#include "CameraManager.hpp"
#include <opencv2/imgcodecs.hpp>
#include "area_scan_3d_camera/api_util.h"
#include "ConfigHelper.hpp"
#include "CameraInfo.hpp"

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

    CameraInfo::getInstance().InitCameraInfo(camera_);
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
        Capture(camera_, suffix);
    }

    return true;
}

bool CameraManager::Stop()
{
    is_running_ = false;
    camera_.disconnect();
    return true;
}

void CameraManager::Capture(mmind::eye::Camera& camera, const std::string& suffix)
{
    if (!is_running_)
        return;

    mmind::eye::Frame2DAnd3D frame2DAnd3D;
    showError(camera.capture2DAnd3D(frame2DAnd3D));

    FrameSet frameSet(frame2DAnd3D, suffix);
    frameSet.DecodeFrame();

    SaveImages(frameSet, suffix);
    ShowImages(frameSet);
}

void CameraManager::SaveImages(FrameSet& frame, const std::string& suffix)
{
    std::vector<std::string> file_names;

    // 2D image
    if (ConfigHelper::getInstance().save_config_.save_2d_image && frame.hasColor)
    {
        std::string image_file = ConfigHelper::getInstance().save_config_.save_2d_image_file(suffix);
        cv::imwrite(image_file, frame.color);
        std::cout << "Capture and save the 2D image: " << image_file << std::endl;
        file_names.push_back(image_file); 
    }

    // Depth map
    if (ConfigHelper::getInstance().save_config_.save_depth_map && frame.hasDepth)
    {
        std::string depth_file = ConfigHelper::getInstance().save_config_.save_depth_map_file(suffix);
        cv::imwrite(depth_file, frame.depthImage);
        std::cout << "Capture and save the depth map: " << depth_file << std::endl;
        file_names.push_back(depth_file);
    }

    // Point cloud
    if (ConfigHelper::getInstance().save_config_.save_point_cloud)
    {
        std::string pointCloudFile = ConfigHelper::getInstance().save_config_.save_point_cloud_file(suffix);
        showError(frame.frame2DAnd3D.frame3D().saveUntexturedPointCloud(mmind::eye::FileFormat::PLY, pointCloudFile),
                  "Capture and save the untextured point cloud: " + pointCloudFile);
        file_names.push_back(pointCloudFile);
    }
    if (ConfigHelper::getInstance().save_config_.save_textured_point_cloud)
    {
        std::string texturedPointCloudFile = ConfigHelper::getInstance().save_config_.save_textured_point_cloud_file(suffix);
        showError(frame.frame2DAnd3D.saveTexturedPointCloud(mmind::eye::FileFormat::PLY, texturedPointCloudFile),
                  "Capture and save the textured point cloud: " + texturedPointCloudFile);
        file_names.push_back(texturedPointCloudFile);
    }

    file_queue_.push_back(file_names);
    CheckFilesLimit();
}

void CameraManager::ShowImages(FrameSet& frame)
{

}

void CameraManager::CheckFilesLimit()
{
    if (file_queue_.size() > ConfigHelper::getInstance().save_config_.max_save_count)
    {
        std::vector<std::string> files_name = file_queue_.front();
        for (const auto& file_name : files_name)
        {
            std::remove(file_name.c_str());
        }
        file_queue_.pop_front();
    }
}
