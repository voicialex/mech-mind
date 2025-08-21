
#include "CameraManager.hpp"
#include <chrono>
#include <thread>
#include <opencv2/imgcodecs.hpp>
#include "utils/UtilHelper.h"
#include "configure/ConfigHelper.hpp"
#include "CameraInfo.hpp"
#include "Logger.hpp"
#include "InferenceInterface.hpp"
#include <sys/stat.h>
#include <sys/types.h>

CameraManager::CameraManager() {
  // Create display window
  if (ConfigHelper::getInstance().camera_config_.render.enable) {
    display_window_ = std::make_unique<CVWindow>(ConfigHelper::getInstance().camera_config_.render.window_title, ConfigHelper::getInstance().camera_config_.render.window_width,
                                                 ConfigHelper::getInstance().camera_config_.render.window_height);
    LOG_INFO_STREAM << "Real-time display window initialized";
  }
}

CameraManager::~CameraManager() {
  if (display_window_) {
    display_window_->close();
  }
}

bool CameraManager::Init() {
  // Check if inference is enabled in config, if enabled then automatically enable inference
  if (ConfigHelper::getInstance().inference_config_.enable) {
    if (!EnableInference()) {
      LOG_WARNING_STREAM << "Failed to enable inference, continuing without inference processing";
    } else {
      LOG_INFO_STREAM << "Inference enabled, algorithm: " << InferenceManager::getInstance().GetCurrentAlgorithmName();
    }
  } else {
    LOG_INFO_STREAM << "Inference not enabled in config, skipping inference processing";
  }

  return true;
}

bool CameraManager::Connect() {
  if (!FindAndConnect(camera_)) return false;

  mmind::eye::CameraInfo cameraInfo;
  showError(camera_.getCameraInfo(cameraInfo));
  printCameraInfo(cameraInfo);

  CameraInfo::getInstance().InitCameraInfo(camera_);
  is_running_ = true;
  return true;
}

bool CameraManager::Start() {
  if (!is_running_) return false;

  LOG_INFO_STREAM << "Starting capture loop...";
  if (display_window_) {
    LOG_INFO_STREAM << "Real-time display enabled. Press ESC to stop.";
  }

  while (is_running_) {
    std::string suffix = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

    // Capture data
    Capture(camera_, suffix);

    // Process window events
    if (display_window_ && !display_window_->processEvents()) {
      LOG_INFO_STREAM << "Window closed by user, stopping capture...";
      break;
    }
  }

  return true;
}

bool CameraManager::Stop() {
  is_running_ = false;
  camera_.disconnect();
  return true;
}

void CameraManager::Capture(mmind::eye::Camera &camera, const std::string &suffix) {
  if (!is_running_) return;

  mmind::eye::Frame2DAnd3D frame2DAnd3D;
  showError(camera.capture2DAnd3D(frame2DAnd3D));

  FrameSet frameSet(frame2DAnd3D, suffix);
  frameSet.DecodeFrame();

  // Process inference
  ProcessInference(frameSet);

  // Save files
  SaveImages(frameSet, suffix);
  // Display images
  ShowImages(frameSet);
}

void CameraManager::SaveImages(FrameSet &frame, const std::string &suffix) {
  // Ensure save directory exists
  std::string save_dir = ConfigHelper::getInstance().camera_config_.save.save_path;
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
  if (ConfigHelper::getInstance().camera_config_.save.save_2d_image && frame.hasColor) {
    std::string image_file = ConfigHelper::getInstance().camera_config_.save.save_2d_image_file(suffix);
    if (cv::imwrite(image_file, frame.color)) {
      LOG_INFO_STREAM << "Capture and save the 2D image: " << image_file;
      file_names.push_back(image_file);
    } else {
      LOG_ERROR_STREAM << "Failed to save 2D image: " << image_file;
    }
  }

  // Depth map
  if (ConfigHelper::getInstance().camera_config_.save.save_depth_map && frame.hasDepth) {
    std::string depth_file = ConfigHelper::getInstance().camera_config_.save.save_depth_map_file(suffix);
    if (cv::imwrite(depth_file, frame.depthImage)) {
      LOG_INFO_STREAM << "Capture and save the depth map: " << depth_file;
      file_names.push_back(depth_file);
    } else {
      LOG_ERROR_STREAM << "Failed to save depth map: " << depth_file;
    }
  }

  // Point cloud
  if (ConfigHelper::getInstance().camera_config_.save.save_point_cloud) {
    std::string pointCloudFile = ConfigHelper::getInstance().camera_config_.save.save_point_cloud_file(suffix);
    showError(frame.frame2DAnd3D.frame3D().saveUntexturedPointCloud(mmind::eye::FileFormat::PLY, pointCloudFile), "Capture and save the untextured point cloud: " + pointCloudFile);
    file_names.push_back(pointCloudFile);
  }
  if (ConfigHelper::getInstance().camera_config_.save.save_textured_point_cloud) {
    std::string texturedPointCloudFile = ConfigHelper::getInstance().camera_config_.save.save_textured_point_cloud_file(suffix);
    showError(frame.frame2DAnd3D.saveTexturedPointCloud(mmind::eye::FileFormat::PLY, texturedPointCloudFile), "Capture and save the textured point cloud: " + texturedPointCloudFile);
    file_names.push_back(texturedPointCloudFile);
  }

  file_queue_.push_back(file_names);
  CheckFilesLimit();
}

void CameraManager::ShowImages(FrameSet &frame) {
  // Display images
  if (display_window_ && ConfigHelper::getInstance().camera_config_.render.enable && frame.hasColor && frame.hasDepth) {
    display_window_->showFrame2DAnd3D(frame.frame2DAnd3D);
  }
}

void CameraManager::CheckFilesLimit() {
  if (file_queue_.size() > ConfigHelper::getInstance().camera_config_.save.max_save_count) {
    std::vector<std::string> files_name = file_queue_.front();
    for (const auto &file_name : files_name) {
      std::remove(file_name.c_str());
    }
    file_queue_.pop_front();
    LOG_DEBUG_STREAM << "Removed old files, current queue size: " << file_queue_.size();
  }
}

bool CameraManager::EnableInference(const std::string &config_path) {
  if (inference_enabled_) {
    LOG_WARNING_STREAM << "Inference already enabled";
    return true;
  }

  // Check if inference is enabled in config
  if (!ConfigHelper::getInstance().inference_config_.enable) {
    LOG_WARNING_STREAM << "Inference not enabled in config, please enable it in config file first";
    return false;
  }

  // Determine config file path
  std::string actual_config_path = config_path.empty() ? ConfigHelper::getInstance().inference_config_.config_path : config_path;

  LOG_INFO_STREAM << "Enabling inference, config file: " << actual_config_path;

  if (InferenceManager::getInstance().InitializeInference(actual_config_path)) {
    inference_enabled_ = true;
    LOG_INFO_STREAM << "Inference enabled successfully, algorithm: " << InferenceManager::getInstance().GetCurrentAlgorithmName();
    return true;
  } else {
    LOG_ERROR_STREAM << "Failed to enable inference";
    return false;
  }
}

void CameraManager::DisableInference() {
  if (inference_enabled_) {
    InferenceManager::getInstance().Cleanup();
    inference_enabled_ = false;
    LOG_INFO_STREAM << "Inference disabled";
  } else {
    LOG_INFO_STREAM << "Inference not enabled, no need to disable";
  }
}

std::string CameraManager::GetInferenceResult() const {
  if (!inference_enabled_) {
    return "Inference not enabled";
  }
  return InferenceManager::getInstance().GetResult();
}

void CameraManager::ProcessInference(FrameSet &frame_set) {
  if (!inference_enabled_) {
    return;
  }

  if (!InferenceManager::getInstance().Process(frame_set)) {
    LOG_WARNING_STREAM << "Failed to process frame with inference";
  }
}
