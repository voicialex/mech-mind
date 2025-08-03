
#include "CameraManager.hpp"
#include <chrono>
#include <thread>
#include <opencv2/imgcodecs.hpp>
// #include "area_scan_3d_camera/api_util.h"
#include "utils/UtilHelper.h"
#include "ConfigHelper.hpp"
#include "CameraInfo.hpp"
#include "utils/Logger.hpp"
#include <sys/stat.h>
#include <sys/types.h>

CameraManager::CameraManager()
{
    // 创建显示窗口
    if (ConfigHelper::getInstance().render_config_.enable) {
        display_window_ = std::make_unique<CVWindow>(
            ConfigHelper::getInstance().render_config_.window_title,
            ConfigHelper::getInstance().render_config_.window_width,
            ConfigHelper::getInstance().render_config_.window_height
        );
        LOG_INFO_STREAM << "Real-time display window initialized";
    }
}

CameraManager::~CameraManager()
{
    if (display_window_) {
        display_window_->close();
    }
}

bool CameraManager::Init()
{
    return true;
}

bool CameraManager::Connect()
{
    if (!FindAndConnect(camera_))
        return false;

    mmind::eye::CameraInfo cameraInfo;
    showError(camera_.getCameraInfo(cameraInfo));
    printCameraInfo(cameraInfo);


    CameraInfo::getInstance().InitCameraInfo(camera_);
    is_running_ = true;
    return true;
}

bool CameraManager::Start()
{
    if (!is_running_)
        return false;

    LOG_INFO_STREAM << "Starting capture loop...";
    if (display_window_) {
        LOG_INFO_STREAM << "Real-time display enabled. Press ESC to stop.";
    }

    while (is_running_)
    {
        std::string suffix = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

        // 采集数据
        Capture(camera_, suffix);

        // 处理窗口事件
        if (display_window_ && !display_window_->processEvents()) {
            LOG_INFO_STREAM << "Window closed by user, stopping capture...";
            break;
        }
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

    // 保存文件
    SaveImages(frameSet, suffix);
    // 显示图像
    ShowImages(frameSet);
}

void CameraManager::SaveImages(FrameSet& frame, const std::string& suffix)
{
    // 确保保存目录存在
    std::string save_dir = ConfigHelper::getInstance().save_config_.save_path;
    struct stat st = {0};
    if (stat(save_dir.c_str(), &st) == -1) {
        if (mkdir(save_dir.c_str(), 0755) == 0) {
            LOG_INFO_STREAM << "Created directory: " << save_dir;
        } else {
            LOG_ERROR_STREAM << "Failed to create directory: " << save_dir;
            return;
        }
    } else {
        LOG_DEBUG_STREAM << "Directory already exists: " << save_dir;
    }
    
    std::vector<std::string> file_names;

    // 2D image
    if (ConfigHelper::getInstance().save_config_.save_2d_image && frame.hasColor)
    {
        std::string image_file = ConfigHelper::getInstance().save_config_.save_2d_image_file(suffix);
        if (cv::imwrite(image_file, frame.color)) {
            LOG_INFO_STREAM << "Capture and save the 2D image: " << image_file;
            file_names.push_back(image_file);
        } else {
            LOG_ERROR_STREAM << "Failed to save 2D image: " << image_file;
        }
    }

    // Depth map
    if (ConfigHelper::getInstance().save_config_.save_depth_map && frame.hasDepth)
    {
        std::string depth_file = ConfigHelper::getInstance().save_config_.save_depth_map_file(suffix);
        if (cv::imwrite(depth_file, frame.depthImage)) {
            LOG_INFO_STREAM << "Capture and save the depth map: " << depth_file;
            file_names.push_back(depth_file);
        } else {
            LOG_ERROR_STREAM << "Failed to save depth map: " << depth_file;
        }
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
    // 显示图像
    if (display_window_ && 
        ConfigHelper::getInstance().render_config_.enable &&
        frame.hasColor && frame.hasDepth) {
        display_window_->showFrame2DAnd3D(frame.frame2DAnd3D);
    }
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
        LOG_DEBUG_STREAM << "Removed old files, current queue size: " << file_queue_.size();
    }
}
