#include "ExampleInference.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

ExampleInference::ExampleInference()
    : is_initialized_(false),
      last_result_("No result available"),
      processed_frame_count_(0),
      confidence_threshold_(0.5),
      min_object_size_(100),
      model_path_("") {}

ExampleInference::~ExampleInference() { Cleanup(); }

bool ExampleInference::Initialize(const std::string &config_path) {
  if (is_initialized_) {
    LOG_WARNING_STREAM << "ExampleInference already initialized";
    return true;
  }

  try {
    // Read config file
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
      LOG_WARNING_STREAM << "Could not open config file: " << config_path << ", using default values";
    } else {
      nlohmann::json config;
      config_file >> config;

      // Read config parameters
      if (config.contains("confidence_threshold")) {
        confidence_threshold_ = config["confidence_threshold"];
      }
      if (config.contains("min_object_size")) {
        min_object_size_ = config["min_object_size"];
      }
      if (config.contains("model_path")) {
        model_path_ = config["model_path"];
      }

      LOG_INFO_STREAM << "Loaded config from: " << config_path;
    }

    // Here you can add actual model loading logic
    // For example: load deep learning model, initialize inference engine, etc.

    is_initialized_ = true;
    processed_frame_count_ = 0;
    last_result_ = "Initialized successfully";

    LOG_INFO_STREAM << "ExampleInference initialized successfully";
    LOG_INFO_STREAM << "Config: confidence_threshold=" << confidence_threshold_
                    << ", min_object_size=" << min_object_size_ << ", model_path=" << model_path_;

    return true;
  } catch (const std::exception &e) {
    LOG_ERROR_STREAM << "Failed to initialize ExampleInference: " << e.what();
    return false;
  }
}

bool ExampleInference::Process(const FrameSet &frame_set) {
  if (!is_initialized_) {
    LOG_ERROR_STREAM << "ExampleInference not initialized";
    return false;
  }

  try {
    std::stringstream result_stream;
    result_stream << "Frame " << processed_frame_count_ << " processed: ";

    // Process 2D color image
    if (frame_set.hasColor) {
      std::string color_result = ProcessColorImage(frame_set.color);
      result_stream << "Color: " << color_result << "; ";
    }

    // Process depth image
    if (frame_set.hasDepth) {
      std::string depth_result = ProcessDepthImage(frame_set.depthImage);
      result_stream << "Depth: " << depth_result << "; ";
    }

    // Process point cloud data
    if (frame_set.hasPointCloud) {
      std::string pointcloud_result = ProcessPointCloud(frame_set.pointCloud);
      result_stream << "PointCloud: " << pointcloud_result << "; ";
    }

    last_result_ = result_stream.str();
    processed_frame_count_++;

    LOG_DEBUG_STREAM << "Processed frame " << processed_frame_count_ << " with ExampleInference";
    return true;
  } catch (const std::exception &e) {
    LOG_ERROR_STREAM << "Failed to process frame with ExampleInference: " << e.what();
    return false;
  }
}

std::string ExampleInference::GetResult() const { return last_result_; }

void ExampleInference::Cleanup() {
  if (is_initialized_) {
    // Here you can add actual cleanup logic
    // For example: release model resources, close inference engine, etc.

    is_initialized_ = false;
    last_result_ = "Cleaned up";
    LOG_INFO_STREAM << "ExampleInference cleaned up";
  }
}

bool ExampleInference::IsInitialized() const { return is_initialized_; }

std::string ExampleInference::GetAlgorithmName() const { return "ExampleInference"; }

std::string ExampleInference::ProcessColorImage(const cv::Mat &color_image) {
  if (color_image.empty()) {
    return "Empty image";
  }

  // Example: simple image processing
  cv::Mat gray_image;
  cv::cvtColor(color_image, gray_image, cv::COLOR_BGR2GRAY);

  // Calculate image statistics
  cv::Scalar mean_val = cv::mean(gray_image);
  cv::Scalar stddev_val, mean_val2;
  cv::meanStdDev(gray_image, mean_val2, stddev_val);

  std::stringstream result;
  result << "Size: " << color_image.size() << ", Mean: " << mean_val[0] << ", Channels: " << color_image.channels();

  return result.str();
}

std::string ExampleInference::ProcessDepthImage(const cv::Mat &depth_image) {
  if (depth_image.empty()) {
    return "Empty depth image";
  }

  // Example: depth image processing
  double min_val, max_val;
  cv::minMaxLoc(depth_image, &min_val, &max_val);

  // Calculate valid depth pixel count
  cv::Mat valid_mask = (depth_image > 0);
  int valid_pixels = cv::countNonZero(valid_mask);

  std::stringstream result;
  result << "Size: " << depth_image.size() << ", Min: " << min_val << ", Max: " << max_val
         << ", Valid pixels: " << valid_pixels;

  return result.str();
}

std::string ExampleInference::ProcessPointCloud(const mmind::eye::PointCloud &point_cloud) {
  if (point_cloud.isEmpty()) {
    return "Empty point cloud";
  }

  // Example: point cloud processing
  size_t point_count = point_cloud.width() * point_cloud.height();

  // Calculate point cloud bounding box
  float min_x = std::numeric_limits<float>::max();
  float max_x = std::numeric_limits<float>::lowest();
  float min_y = std::numeric_limits<float>::max();
  float max_y = std::numeric_limits<float>::lowest();
  float min_z = std::numeric_limits<float>::max();
  float max_z = std::numeric_limits<float>::lowest();

  // Iterate through point cloud data
  for (size_t i = 0; i < point_count; ++i) {
    const auto &point = point_cloud[i];
    min_x = std::min(min_x, point.x);
    max_x = std::max(max_x, point.x);
    min_y = std::min(min_y, point.y);
    max_y = std::max(max_y, point.y);
    min_z = std::min(min_z, point.z);
    max_z = std::max(max_z, point.z);
  }

  std::stringstream result;
  result << "Points: " << point_count << ", BBox: [" << min_x << "," << min_y << "," << min_z << "] to [" << max_x
         << "," << max_y << "," << max_z << "]";

  return result.str();
}
