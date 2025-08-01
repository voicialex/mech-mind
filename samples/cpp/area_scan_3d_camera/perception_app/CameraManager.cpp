
#include "CameraManager.hpp"
#include <chrono>
#include <thread>
#include <memory>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include "area_scan_3d_camera/api_util.h"
#include "ConfigHelper.hpp"
#include "CameraInfo.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <iomanip> // Required for std::fixed and std::setprecision

CameraManager::CameraManager()
{
    // 根据配置初始化显示窗口
    if (ConfigHelper::getInstance().render_config_.enable && 
        ConfigHelper::getInstance().render_config_.show_real_time) {
        display_window_ = std::make_unique<CVWindow>(
            ConfigHelper::getInstance().render_config_.window_title,
            ConfigHelper::getInstance().render_config_.window_width,
            ConfigHelper::getInstance().render_config_.window_height
        );
        std::cout << "Real-time display window initialized." << std::endl;
    }
    
    // 初始化性能监控
    frame_count_ = 0;
    last_frame_time_ = std::chrono::high_resolution_clock::now();
    performance_start_time_ = last_frame_time_;
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

    std::cout << "Starting capture loop..." << std::endl;
    if (display_window_) {
        std::cout << "Real-time display enabled. Press ESC to stop." << std::endl;
    }

    while (is_running_)
    {
        auto frame_start_time = std::chrono::high_resolution_clock::now();
        
        std::string suffix = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        
        // 采集数据
        Capture(camera_, suffix);
        
        // 立即处理窗口事件，确保响应性
        if (display_window_ && !ProcessWindowEvents()) {
            std::cout << "Window closed by user, stopping capture..." << std::endl;
            break;
        }
        
        // 性能监控
        if (ConfigHelper::getInstance().render_config_.enable_performance_monitor) {
            frame_count_++;
            auto frame_end_time = std::chrono::high_resolution_clock::now();
            
            if (frame_count_ % ConfigHelper::getInstance().render_config_.performance_report_interval == 0) {
                auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    frame_end_time - performance_start_time_).count();
                auto frame_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    frame_end_time - frame_start_time).count();
                
                double avg_fps = 1000.0 * frame_count_ / total_duration;
                std::cout << "Performance Report: Frame " << frame_count_ 
                         << ", Current frame time: " << frame_duration << "ms"
                         << ", Average FPS: " << std::fixed << std::setprecision(2) << avg_fps << std::endl;
            }
        }
        
        // 减少延迟，提高响应性
        std::this_thread::sleep_for(std::chrono::milliseconds(
            ConfigHelper::getInstance().render_config_.display_refresh_ms));
    }

    // 等待所有异步保存任务完成
    std::cout << "Waiting for async save tasks to complete..." << std::endl;
    WaitForAllSaves();

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

    // 优先显示图像，确保实时性
    ShowImages(frameSet);
    
    // 异步保存文件，不阻塞显示
    SaveImagesAsync(frameSet, suffix);
    
    // 清理已完成的异步任务
    CleanupCompletedSaves();
}

void CameraManager::SaveImages(FrameSet& frame, const std::string& suffix)
{
    // 确保保存目录存在
    std::string save_dir = ConfigHelper::getInstance().save_config_.save_path;
    struct stat st = {0};
    if (stat(save_dir.c_str(), &st) == -1) {
        if (mkdir(save_dir.c_str(), 0755) == 0) {
            std::cout << "Created directory: " << save_dir << std::endl;
        } else {
            std::cerr << "Failed to create directory: " << save_dir << std::endl;
            return;
        }
    } else {
        std::cout << "Directory already exists: " << save_dir << std::endl;
    }
    
    std::vector<std::string> file_names;

    // 2D image
    if (ConfigHelper::getInstance().save_config_.save_2d_image && frame.hasColor)
    {
        std::string image_file = ConfigHelper::getInstance().save_config_.save_2d_image_file(suffix);
        if (cv::imwrite(image_file, frame.color)) {
        std::cout << "Capture and save the 2D image: " << image_file << std::endl;
        file_names.push_back(image_file); 
        } else {
            std::cerr << "Failed to save 2D image: " << image_file << std::endl;
        }
    }

    // Depth map
    if (ConfigHelper::getInstance().save_config_.save_depth_map && frame.hasDepth)
    {
        std::string depth_file = ConfigHelper::getInstance().save_config_.save_depth_map_file(suffix);
        if (cv::imwrite(depth_file, frame.depthImage)) {
        std::cout << "Capture and save the depth map: " << depth_file << std::endl;
        file_names.push_back(depth_file);
        } else {
            std::cerr << "Failed to save depth map: " << depth_file << std::endl;
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
    // 只在启用显示时显示图像
    if (display_window_ && 
        ConfigHelper::getInstance().render_config_.enable &&
        ConfigHelper::getInstance().render_config_.show_real_time &&
        frame.hasColor && frame.hasDepth) {
        display_window_->showFrame2DAnd3D(frame.frame2DAnd3D);
    }
}

bool CameraManager::ProcessWindowEvents()
{
    if (display_window_) {
        return display_window_->processEvents();
    }
    return true;
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

void CameraManager::SaveImagesAsync(FrameSet frame, const std::string suffix)
{
    // 启动异步任务保存文件
    auto future = std::async(std::launch::async, [this, frame, suffix]() {
        this->SaveImages(const_cast<FrameSet&>(frame), suffix);
    });
    
    // 存储future用于后续清理
    {
        std::lock_guard<std::mutex> lock(save_mutex_);
        save_futures_.push(std::move(future));
    }
}

void CameraManager::CleanupCompletedSaves()
{
    std::lock_guard<std::mutex> lock(save_mutex_);
    
    // 清理已完成的保存任务
    while (!save_futures_.empty()) {
        auto& future = save_futures_.front();
        if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            try {
                future.get(); // 获取结果，处理可能的异常
            } catch (const std::exception& e) {
                std::cerr << "Async save error: " << e.what() << std::endl;
            }
            save_futures_.pop();
        } else {
            break; // 如果当前任务未完成，后续任务也不会完成
        }
    }
}

void CameraManager::WaitForAllSaves()
{
    std::lock_guard<std::mutex> lock(save_mutex_);
    
    // 等待所有异步保存任务完成
    while (!save_futures_.empty()) {
        auto& future = save_futures_.front();
        try {
            future.get(); // 等待任务完成
            std::cout << "Async save task completed." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Async save error: " << e.what() << std::endl;
        }
        save_futures_.pop();
    }
    std::cout << "All async save tasks completed." << std::endl;
}
