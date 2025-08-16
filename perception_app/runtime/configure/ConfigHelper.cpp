#include "ConfigHelper.hpp"
#include <iostream>

std::string ConfigHelper::SaveConfig::save_2d_image_file(const std::string &suffix) const { return this->save_path + "/" + suffix + "_2DImage.png"; }

std::string ConfigHelper::SaveConfig::save_depth_map_file(const std::string &suffix) const { return this->save_path + "/" + suffix + "_DepthMap.tiff"; }

std::string ConfigHelper::SaveConfig::save_point_cloud_file(const std::string &suffix) const { return this->save_path + "/" + suffix + "_PointCloud.ply"; }

std::string ConfigHelper::SaveConfig::save_textured_point_cloud_file(const std::string &suffix) const { return this->save_path + "/" + suffix + "_TexturedPointCloud.ply"; }

bool ConfigHelper::loadConfigFromJson(const std::string &configPath) {
  try {
    std::ifstream file(configPath);
    if (!file.is_open()) {
      std::cerr << "Cannot open config file: " << configPath << std::endl;
      return false;
    }

    nlohmann::json j;
    file >> j;

    // Parse render config
    if (j.contains("render_config")) {
      auto &render = j["render_config"];
      render_config_.enable = render.value("enable", true);
      render_config_.window_title = render.value("window_title", "Real-time Capture Display");
      render_config_.window_width = render.value("window_width", 1280);
      render_config_.window_height = render.value("window_height", 720);
    }

    // Parse capture config
    if (j.contains("capture_config")) {
      auto &capture = j["capture_config"];
      capture_config_.enable = capture.value("enable", true);
      capture_config_.suffix = capture.value("suffix", "");
      capture_config_.color_file = capture.value("color_file", "2DImage.png");
      capture_config_.depth_file = capture.value("depth_file", "DepthMap.tiff");
      capture_config_.point_cloud_file = capture.value("point_cloud_file", "PointCloud.ply");
      capture_config_.textured_point_cloud_file = capture.value("textured_point_cloud_file", "TexturedPointCloud.ply");
    }

    // Parse save config
    if (j.contains("save_config")) {
      auto &save = j["save_config"];
      save_config_.save_path = save.value("save_path", "./dumps/");
      save_config_.save_2d_image = save.value("save_2d_image", true);
      save_config_.save_depth_map = save.value("save_depth_map", true);
      save_config_.save_point_cloud = save.value("save_point_cloud", true);
      save_config_.save_textured_point_cloud = save.value("save_textured_point_cloud", true);
      save_config_.max_save_count = save.value("max_save_count", 20);
    }

    // Parse log config
    if (j.contains("log_config")) {
      auto &log = j["log_config"];
      log_config_.enable = log.value("enable", true);
      log_config_.level = log.value("level", "INFO");
      log_config_.show_timestamp = log.value("show_timestamp", true);
      log_config_.show_level = log.value("show_level", true);
    }

    // Parse inference config
    if (j.contains("inference_config")) {
      auto &inference = j["inference_config"];
      inference_config_.enable = inference.value("enable", false);
      inference_config_.config_path = inference.value("config_path", "config/inference_config.json");
    }

    std::cout << "Config file loaded successfully: " << configPath << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error parsing config file: " << e.what() << std::endl;
    return false;
  }
}

void ConfigHelper::printConfig() const {
  std::cout << "=== Current Configuration ===" << std::endl;

  std::cout << "Render Config:" << std::endl;
  std::cout << "  Enabled: " << (render_config_.enable ? "Yes" : "No") << std::endl;
  std::cout << "  Window Title: " << render_config_.window_title << std::endl;
  std::cout << "  Window Size: " << render_config_.window_width << "x" << render_config_.window_height << std::endl;

  std::cout << "Capture Config:" << std::endl;
  std::cout << "  Enabled: " << (capture_config_.enable ? "Yes" : "No") << std::endl;
  std::cout << "  Suffix: " << capture_config_.suffix << std::endl;
  std::cout << "  Color File: " << capture_config_.color_file << std::endl;
  std::cout << "  Depth File: " << capture_config_.depth_file << std::endl;
  std::cout << "  Point Cloud File: " << capture_config_.point_cloud_file << std::endl;
  std::cout << "  Textured Point Cloud File: " << capture_config_.textured_point_cloud_file << std::endl;

  std::cout << "Save Config:" << std::endl;
  std::cout << "  Save Path: " << save_config_.save_path << std::endl;
  std::cout << "  Save 2D Image: " << (save_config_.save_2d_image ? "Yes" : "No") << std::endl;
  std::cout << "  Save Depth Map: " << (save_config_.save_depth_map ? "Yes" : "No") << std::endl;
  std::cout << "  Save Point Cloud: " << (save_config_.save_point_cloud ? "Yes" : "No") << std::endl;
  std::cout << "  Save Textured Point Cloud: " << (save_config_.save_textured_point_cloud ? "Yes" : "No") << std::endl;
  std::cout << "  Max Save Count: " << save_config_.max_save_count << std::endl;

  std::cout << "Log Config:" << std::endl;
  std::cout << "  Enabled: " << (log_config_.enable ? "Yes" : "No") << std::endl;
  std::cout << "  Level: " << log_config_.level << std::endl;
  std::cout << "  Show Timestamp: " << (log_config_.show_timestamp ? "Yes" : "No") << std::endl;
  std::cout << "  Show Level: " << (log_config_.show_level ? "Yes" : "No") << std::endl;

  std::cout << "Inference Config:" << std::endl;
  std::cout << "  Enabled: " << (inference_config_.enable ? "Yes" : "No") << std::endl;
  std::cout << "  Config Path: " << inference_config_.config_path << std::endl;

  std::cout << "==================" << std::endl;
}