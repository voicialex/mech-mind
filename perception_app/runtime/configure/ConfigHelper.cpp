#include "ConfigHelper.hpp"
#include <iostream>

namespace {
// 解析communication配置的通用函数（用于内联对象解析）
static void ParseCommunicationJson(ConfigHelper::CommunicationConfig &cfg, const nlohmann::json &j) {
  if (j.contains("service_discovery")) {
    auto &sd = j["service_discovery"];
    cfg.service_discovery.local_address = sd.value("local_address", "0.0.0.0");
    cfg.service_discovery.discovery_port = sd.value("discovery_port", 8080);
    cfg.service_discovery.broadcast_interval = sd.value("broadcast_interval", 5000);
    cfg.service_discovery.enable_broadcast = sd.value("enable_broadcast", true);
  }

  if (j.contains("server")) {
    auto &server = j["server"];
    cfg.server.id = server.value("id", "device_server");
    cfg.server.name = server.value("name", "Device Server");
    cfg.server.address = server.value("address", "127.0.0.1");
    cfg.server.port = server.value("port", 9090);
    cfg.server.max_clients = server.value("max_clients", 100);
    cfg.server.client_timeout = server.value("client_timeout", 30000);
  }

  if (j.contains("client")) {
    auto &client = j["client"];
    cfg.client.id = client.value("id", "device_client");
    cfg.client.name = client.value("name", "Device Client");
    cfg.client.address = client.value("address", "0.0.0.0");
    cfg.client.port = client.value("port", 0);
    cfg.client.enable_auto_reconnect = client.value("enable_auto_reconnect", true);
    cfg.client.max_reconnect_attempts = client.value("max_reconnect_attempts", 10);
    cfg.client.reconnect_interval = client.value("reconnect_interval", 5000);
    cfg.client.connection_check_interval = client.value("connection_check_interval", 10000);
  }

  if (j.contains("heartbeat")) {
    auto &heartbeat = j["heartbeat"];
    cfg.heartbeat.enable = heartbeat.value("enable", true);
    cfg.heartbeat.interval = heartbeat.value("interval", 30000);
    cfg.heartbeat.timeout_multiplier = heartbeat.value("timeout_multiplier", 3);
    cfg.heartbeat.max_missed_responses = heartbeat.value("max_missed_responses", 3);
  }

  if (j.contains("master_node")) {
    auto &master = j["master_node"];
    cfg.master_node.client_timeout_interval = master.value("client_timeout_interval", 60000);
    cfg.master_node.status_check_interval = master.value("status_check_interval", 10000);
    cfg.master_node.state_sync_interval = master.value("state_sync_interval", 5000);
    cfg.master_node.enable_auto_cleanup = master.value("enable_auto_cleanup", true);
  }

  if (j.contains("message")) {
    auto &message = j["message"];
    cfg.message.max_payload_size = message.value("max_payload_size", 65536);
    cfg.message.message_timeout = message.value("message_timeout", 5000);
    cfg.message.enable_crc_check = message.value("enable_crc_check", true);
  }
}
}  // namespace

std::string ConfigHelper::CameraConfig::SaveConfig::save_2d_image_file(const std::string &suffix) const {
  return this->save_path + "/" + suffix + "_2DImage.png";
}

std::string ConfigHelper::CameraConfig::SaveConfig::save_depth_map_file(const std::string &suffix) const {
  return this->save_path + "/" + suffix + "_DepthMap.tiff";
}

std::string ConfigHelper::CameraConfig::SaveConfig::save_point_cloud_file(const std::string &suffix) const {
  return this->save_path + "/" + suffix + "_PointCloud.ply";
}

std::string ConfigHelper::CameraConfig::SaveConfig::save_textured_point_cloud_file(const std::string &suffix) const {
  return this->save_path + "/" + suffix + "_TexturedPointCloud.ply";
}

bool ConfigHelper::loadConfigFromJson(const std::string &configPath) {
  try {
    std::ifstream file(configPath);
    if (!file.is_open()) {
      std::cerr << "Cannot open config file: " << configPath << std::endl;
      return false;
    }

    nlohmann::json j;
    file >> j;

    // Parse camera config
    if (j.contains("camera_config")) {
      auto &camera = j["camera_config"];

      // Parse render config
      if (camera.contains("render")) {
        auto &render = camera["render"];
        camera_config_.render.enable = render.value("enable", true);
        camera_config_.render.window_title = render.value("window_title", "Real-time Capture Display");
        camera_config_.render.window_width = render.value("window_width", 1280);
        camera_config_.render.window_height = render.value("window_height", 720);
      }

      // Parse capture config
      if (camera.contains("capture")) {
        auto &capture = camera["capture"];
        camera_config_.capture.enable = capture.value("enable", true);
        camera_config_.capture.suffix = capture.value("suffix", "");
        camera_config_.capture.color_file = capture.value("color_file", "2DImage.png");
        camera_config_.capture.depth_file = capture.value("depth_file", "DepthMap.tiff");
        camera_config_.capture.point_cloud_file = capture.value("point_cloud_file", "PointCloud.ply");
        camera_config_.capture.textured_point_cloud_file =
            capture.value("textured_point_cloud_file", "TexturedPointCloud.ply");
      }

      // Parse save config
      if (camera.contains("save")) {
        auto &save = camera["save"];
        camera_config_.save.save_path = save.value("save_path", "./dumps/");
        camera_config_.save.save_2d_image = save.value("save_2d_image", true);
        camera_config_.save.save_depth_map = save.value("save_depth_map", true);
        camera_config_.save.save_point_cloud = save.value("save_point_cloud", true);
        camera_config_.save.save_textured_point_cloud = save.value("save_textured_point_cloud", true);
        camera_config_.save.max_save_count = save.value("max_save_count", 20);
      }
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

    // Parse communication config (single entry point)
    if (j.contains("communication_config")) {
      const auto &comm = j["communication_config"];
      if (comm.is_string()) {
        // 支持相对路径：以主配置文件所在目录为根
        std::string commPath = comm.get<std::string>();
        std::string resolved = commPath;
        if (!commPath.empty() && commPath[0] != '/') {
          auto pos = configPath.find_last_of('/');
          if (pos != std::string::npos) {
            resolved = configPath.substr(0, pos + 1) + commPath;
          }
        }
        // 读取独立的communication配置文件
        if (!loadCommunicationConfigFromJson(resolved)) {
          std::cerr << "Failed to load communication config from: " << resolved << std::endl;
        }
      } else if (comm.is_object()) {
        // 解析嵌入式communication配置
        ParseCommunicationJson(communication_config_, comm);
        std::cout << "Communication config parsed from embedded object." << std::endl;
      } else {
        std::cerr << "Invalid communication_config format in: " << configPath << std::endl;
      }
    }

    std::cout << "Config file loaded successfully: " << configPath << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error parsing config file: " << e.what() << std::endl;
    return false;
  }
}

bool ConfigHelper::loadCommunicationConfigFromJson(const std::string &configPath) {
  try {
    std::ifstream file(configPath);
    if (!file.is_open()) {
      std::cerr << "Cannot open communication config file: " << configPath << std::endl;
      return false;
    }

    nlohmann::json j;
    file >> j;

    // Parse service discovery config
    if (j.contains("service_discovery")) {
      auto &sd = j["service_discovery"];
      communication_config_.service_discovery.local_address = sd.value("local_address", "0.0.0.0");
      communication_config_.service_discovery.discovery_port = sd.value("discovery_port", 8080);
      communication_config_.service_discovery.broadcast_interval = sd.value("broadcast_interval", 5000);
      communication_config_.service_discovery.enable_broadcast = sd.value("enable_broadcast", true);
    }

    // Parse server config
    if (j.contains("server")) {
      auto &server = j["server"];
      communication_config_.server.id = server.value("id", "device_server");
      communication_config_.server.name = server.value("name", "Device Server");
      communication_config_.server.address = server.value("address", "127.0.0.1");
      communication_config_.server.port = server.value("port", 9090);
      communication_config_.server.max_clients = server.value("max_clients", 100);
      communication_config_.server.client_timeout = server.value("client_timeout", 30000);
    }

    // Parse client config
    if (j.contains("client")) {
      auto &client = j["client"];
      communication_config_.client.id = client.value("id", "device_client");
      communication_config_.client.name = client.value("name", "Device Client");
      communication_config_.client.address = client.value("address", "0.0.0.0");
      communication_config_.client.port = client.value("port", 0);
      communication_config_.client.enable_auto_reconnect = client.value("enable_auto_reconnect", true);
      communication_config_.client.max_reconnect_attempts = client.value("max_reconnect_attempts", 10);
      communication_config_.client.reconnect_interval = client.value("reconnect_interval", 5000);
      communication_config_.client.connection_check_interval = client.value("connection_check_interval", 10000);
    }

    // Parse heartbeat config
    if (j.contains("heartbeat")) {
      auto &heartbeat = j["heartbeat"];
      communication_config_.heartbeat.enable = heartbeat.value("enable", true);
      communication_config_.heartbeat.interval = heartbeat.value("interval", 30000);
      communication_config_.heartbeat.timeout_multiplier = heartbeat.value("timeout_multiplier", 3);
      communication_config_.heartbeat.max_missed_responses = heartbeat.value("max_missed_responses", 3);
    }

    // Parse master node config
    if (j.contains("master_node")) {
      auto &master = j["master_node"];
      communication_config_.master_node.client_timeout_interval = master.value("client_timeout_interval", 60000);
      communication_config_.master_node.status_check_interval = master.value("status_check_interval", 10000);
      communication_config_.master_node.state_sync_interval = master.value("state_sync_interval", 5000);
      communication_config_.master_node.enable_auto_cleanup = master.value("enable_auto_cleanup", true);
    }

    // Parse message config
    if (j.contains("message")) {
      auto &message = j["message"];
      communication_config_.message.max_payload_size = message.value("max_payload_size", 65536);
      communication_config_.message.message_timeout = message.value("message_timeout", 5000);
      communication_config_.message.enable_crc_check = message.value("enable_crc_check", true);
    }

    std::cout << "Communication config file loaded successfully: " << configPath << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "Error parsing communication config file: " << e.what() << std::endl;
    return false;
  }
}

void ConfigHelper::printConfig() const {
  std::cout << "=== Current Configuration ===" << std::endl;

  std::cout << "Camera Config:" << std::endl;
  std::cout << "  Render:" << std::endl;
  std::cout << "    Enabled: " << (camera_config_.render.enable ? "Yes" : "No") << std::endl;
  std::cout << "    Window Title: " << camera_config_.render.window_title << std::endl;
  std::cout << "    Window Size: " << camera_config_.render.window_width << "x" << camera_config_.render.window_height
            << std::endl;

  std::cout << "  Capture:" << std::endl;
  std::cout << "    Enabled: " << (camera_config_.capture.enable ? "Yes" : "No") << std::endl;
  std::cout << "    Suffix: " << camera_config_.capture.suffix << std::endl;
  std::cout << "    Color File: " << camera_config_.capture.color_file << std::endl;
  std::cout << "    Depth File: " << camera_config_.capture.depth_file << std::endl;
  std::cout << "    Point Cloud File: " << camera_config_.capture.point_cloud_file << std::endl;
  std::cout << "    Textured Point Cloud File: " << camera_config_.capture.textured_point_cloud_file << std::endl;

  std::cout << "  Save:" << std::endl;
  std::cout << "    Save Path: " << camera_config_.save.save_path << std::endl;
  std::cout << "    Save 2D Image: " << (camera_config_.save.save_2d_image ? "Yes" : "No") << std::endl;
  std::cout << "    Save Depth Map: " << (camera_config_.save.save_depth_map ? "Yes" : "No") << std::endl;
  std::cout << "    Save Point Cloud: " << (camera_config_.save.save_point_cloud ? "Yes" : "No") << std::endl;
  std::cout << "    Save Textured Point Cloud: " << (camera_config_.save.save_textured_point_cloud ? "Yes" : "No")
            << std::endl;
  std::cout << "    Max Save Count: " << camera_config_.save.max_save_count << std::endl;

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

void ConfigHelper::printCommunicationConfig() const {
  std::cout << "=== Communication Configuration ===" << std::endl;

  std::cout << "Service Discovery Config:" << std::endl;
  std::cout << "  Local Address: " << communication_config_.service_discovery.local_address << std::endl;
  std::cout << "  Discovery Port: " << communication_config_.service_discovery.discovery_port << std::endl;
  std::cout << "  Broadcast Interval: " << communication_config_.service_discovery.broadcast_interval << "ms"
            << std::endl;
  std::cout << "  Enable Broadcast: " << (communication_config_.service_discovery.enable_broadcast ? "Yes" : "No")
            << std::endl;

  std::cout << "Server Config:" << std::endl;
  std::cout << "  ID: " << communication_config_.server.id << std::endl;
  std::cout << "  Name: " << communication_config_.server.name << std::endl;
  std::cout << "  Address: " << communication_config_.server.address << ":" << communication_config_.server.port
            << std::endl;
  std::cout << "  Max Clients: " << communication_config_.server.max_clients << std::endl;
  std::cout << "  Client Timeout: " << communication_config_.server.client_timeout << "ms" << std::endl;

  std::cout << "Client Config:" << std::endl;
  std::cout << "  ID: " << communication_config_.client.id << std::endl;
  std::cout << "  Name: " << communication_config_.client.name << std::endl;
  std::cout << "  Address: " << communication_config_.client.address << ":" << communication_config_.client.port
            << std::endl;
  std::cout << "  Auto Reconnect: " << (communication_config_.client.enable_auto_reconnect ? "Yes" : "No") << std::endl;
  std::cout << "  Max Reconnect Attempts: " << communication_config_.client.max_reconnect_attempts << std::endl;
  std::cout << "  Reconnect Interval: " << communication_config_.client.reconnect_interval << "ms" << std::endl;
  std::cout << "  Connection Check Interval: " << communication_config_.client.connection_check_interval << "ms"
            << std::endl;

  std::cout << "Heartbeat Config:" << std::endl;
  std::cout << "  Enabled: " << (communication_config_.heartbeat.enable ? "Yes" : "No") << std::endl;
  std::cout << "  Interval: " << communication_config_.heartbeat.interval << "ms" << std::endl;
  std::cout << "  Timeout Multiplier: " << communication_config_.heartbeat.timeout_multiplier << std::endl;
  std::cout << "  Max Missed Responses: " << communication_config_.heartbeat.max_missed_responses << std::endl;

  std::cout << "Master Node Config:" << std::endl;
  std::cout << "  Client Timeout Interval: " << communication_config_.master_node.client_timeout_interval << "ms"
            << std::endl;
  std::cout << "  Status Check Interval: " << communication_config_.master_node.status_check_interval << "ms"
            << std::endl;
  std::cout << "  State Sync Interval: " << communication_config_.master_node.state_sync_interval << "ms" << std::endl;
  std::cout << "  Auto Cleanup: " << (communication_config_.master_node.enable_auto_cleanup ? "Yes" : "No")
            << std::endl;

  std::cout << "Message Config:" << std::endl;
  std::cout << "  Max Payload Size: " << communication_config_.message.max_payload_size << " bytes" << std::endl;
  std::cout << "  Message Timeout: " << communication_config_.message.message_timeout << "ms" << std::endl;
  std::cout << "  Enable CRC Check: " << (communication_config_.message.enable_crc_check ? "Yes" : "No") << std::endl;

  std::cout << "==================" << std::endl;
}