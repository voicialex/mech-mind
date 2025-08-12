#include "../include/DeviceClient.hpp"
#include <iostream>
#include <signal.h>
#include <csignal>
#include <memory>

using namespace device_center;

// 全局变量用于信号处理
std::unique_ptr<DeviceClient> g_client;
std::atomic<bool> g_running{true};

// 信号处理函数
void SignalHandler(int signal) {
  std::cout << "\n收到信号 " << signal << "，正在关闭客户端..." << std::endl;
  g_running = false;

  if (g_client) {
    g_client->Stop();
  }
}

// 示例设备处理器
class ExampleDeviceHandler : public DeviceHandler {
 public:
  DeviceResponse HandleCommand(const DeviceCommand &command) override {
    std::cout << "客户端处理设备命令: " << command.device_id << " - " << command.command_type << std::endl;

    DeviceResponse response;
    response.device_id = command.device_id;
    response.command_id = command.command_id;
    response.success = true;
    response.message = "客户端命令执行成功";
    response.timestamp = DeviceUtils::GetCurrentTimestamp();
    response.error_code = 0;

    // 根据命令类型处理
    if (command.command_type == "start") {
      response.data["status"] = "started";
      std::cout << "设备 " << command.device_id << " 已启动" << std::endl;
    } else if (command.command_type == "stop") {
      response.data["status"] = "stopped";
      std::cout << "设备 " << command.device_id << " 已停止" << std::endl;
    } else if (command.command_type == "configure") {
      response.data["config"] = command.parameters;
      std::cout << "设备 " << command.device_id << " 配置已更新" << std::endl;
    }

    return response;
  }

  DeviceStatusInfo GetDeviceStatus(const std::string &device_id) override {
    DeviceStatusInfo status;
    status.device_id = device_id;
    status.status = DeviceStatus::Online;
    status.status_message = "客户端设备正常运行";
    status.timestamp = DeviceUtils::GetCurrentTimestamp();
    status.cpu_usage = 20.0f;
    status.memory_usage = 30.0f;
    status.temperature = 32.0f;
    status.status_data["uptime"] = 1800;
    status.status_data["data_count"] = 500;

    return status;
  }

  DeviceInfo GetDeviceInfo(const std::string &device_id) override {
    DeviceInfo info;
    info.device_id = device_id;
    info.device_name = "客户端示例设备";
    info.device_type = DeviceType::Camera;
    info.device_model = "Client-Camera-001";
    info.device_version = "1.0.0";
    info.device_serial = "SN987654321";
    info.device_ip = "192.168.1.200";
    info.device_port = 8081;
    info.capabilities = {DeviceCapability::Capture, DeviceCapability::Configure, DeviceCapability::Status};
    info.status = DeviceStatus::Online;
    info.status_message = "客户端设备在线";
    info.last_heartbeat = DeviceUtils::GetCurrentTimestamp();
    info.config["resolution"] = "1280x720";
    info.config["fps"] = 25;

    return info;
  }

  std::vector<DeviceType> GetSupportedDeviceTypes() const override { return {DeviceType::Camera, DeviceType::Sensor}; }
};

// 示例事件处理器
class ExampleEventHandler : public DeviceEventHandler {
 public:
  void OnDeviceStatusChanged(const std::string &device_id, DeviceStatus old_status, DeviceStatus new_status) override {
    std::cout << "客户端设备状态变化: " << device_id << " " << DeviceUtils::StatusToString(old_status) << " -> " << DeviceUtils::StatusToString(new_status) << std::endl;
  }

  void OnDeviceDataReceived(const DeviceData &device_data) override {
    std::cout << "客户端收到设备数据: " << device_data.device_id << " 类型: " << device_data.data_type << " 大小: " << device_data.data_size << std::endl;
  }

  void OnDeviceError(const std::string &device_id, uint16_t error_code, const std::string &error_message) override {
    std::cout << "客户端设备错误: " << device_id << " 错误码: " << error_code << " 消息: " << error_message << std::endl;
  }
};

int main(int argc, char *argv[]) {
  std::cout << "=== Device Center Client ===" << std::endl;

  // 设置信号处理
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  // 配置文件路径
  std::string config_path = "config/client_config.json";
  if (argc > 1) {
    config_path = argv[1];
  }

  try {
    // 创建设备客户端
    g_client = std::make_unique<DeviceClient>();

    // 初始化客户端
    std::cout << "正在初始化设备客户端..." << std::endl;
    if (!g_client->Initialize(config_path)) {
      std::cerr << "客户端初始化失败!" << std::endl;
      return -1;
    }

    // 注册设备处理器
    std::cout << "正在注册设备处理器..." << std::endl;
    auto device_handler = std::make_shared<ExampleDeviceHandler>();
    g_client->RegisterDeviceHandler("camera_001", device_handler);
    g_client->RegisterDeviceHandler("sensor_001", device_handler);

    // 注册事件处理器
    auto event_handler = std::make_shared<ExampleEventHandler>();
    g_client->RegisterEventHandler(event_handler);

    // 启动客户端
    std::cout << "正在启动设备客户端..." << std::endl;
    if (!g_client->Start()) {
      std::cerr << "客户端启动失败!" << std::endl;
      return -1;
    }

    // 发现服务器
    std::cout << "正在发现设备服务器..." << std::endl;
    auto servers = g_client->DiscoverServers("Device Management Server");
    if (servers.empty()) {
      std::cout << "未发现设备服务器，使用默认配置连接..." << std::endl;
    } else {
      std::cout << "发现服务器: " << servers[0] << std::endl;
    }

    // 连接到服务器
    std::cout << "正在连接到设备服务器..." << std::endl;
    if (!g_client->ConnectToServer()) {
      std::cerr << "连接服务器失败!" << std::endl;
      return -1;
    }

    std::cout << "设备客户端启动成功!" << std::endl;
    std::cout << "客户端ID: " << g_client->GetConfig().service_id << std::endl;
    std::cout << "服务器地址: " << g_client->GetConfig().server_address << ":" << g_client->GetConfig().server_port << std::endl;
    std::cout << "按 Ctrl+C 停止客户端" << std::endl;

    // 注册示例设备
    std::cout << "正在注册示例设备..." << std::endl;

    // 注册相机设备
    nlohmann::json camera_config;
    camera_config["resolution"] = "1920x1080";
    camera_config["fps"] = 30;
    camera_config["exposure"] = "auto";

    if (g_client->RegisterDevice("camera_001", DeviceType::Camera, camera_config)) {
      std::cout << "相机设备注册成功" << std::endl;

      // 启用心跳
      g_client->EnableDeviceHeartbeat("camera_001", 5000);
    }

    // 注册传感器设备
    nlohmann::json sensor_config;
    sensor_config["sampling_rate"] = 1000;
    sensor_config["calibration_enabled"] = true;

    if (g_client->RegisterDevice("sensor_001", DeviceType::Sensor, sensor_config)) {
      std::cout << "传感器设备注册成功" << std::endl;

      // 启用心跳
      g_client->EnableDeviceHeartbeat("sensor_001", 3000);
    }

    // 运行客户端循环
    while (g_running) {
      std::this_thread::sleep_for(std::chrono::seconds(1));

      // 定期上报设备状态
      static int counter = 0;
      if (++counter % 10 == 0) {  // 每10秒上报一次状态
        DeviceStatusInfo camera_status;
        camera_status.device_id = "camera_001";
        camera_status.status = DeviceStatus::Online;
        camera_status.status_message = "相机正常运行";
        camera_status.timestamp = DeviceUtils::GetCurrentTimestamp();
        camera_status.cpu_usage = 15.0f;
        camera_status.memory_usage = 25.0f;
        camera_status.temperature = 30.0f;

        g_client->ReportDeviceStatus("camera_001", camera_status);

        DeviceStatusInfo sensor_status;
        sensor_status.device_id = "sensor_001";
        sensor_status.status = DeviceStatus::Online;
        sensor_status.status_message = "传感器正常运行";
        sensor_status.timestamp = DeviceUtils::GetCurrentTimestamp();
        sensor_status.cpu_usage = 10.0f;
        sensor_status.memory_usage = 15.0f;
        sensor_status.temperature = 28.0f;

        g_client->ReportDeviceStatus("sensor_001", sensor_status);
      }

      // 定期输出统计信息
      if (counter % 30 == 0) {  // 每30秒输出一次统计信息
        auto stats = g_client->GetClientStatistics();
        std::cout << "客户端统计: " << stats.dump(2) << std::endl;

        auto devices = g_client->GetRegisteredDevices();
        std::cout << "已注册设备数量: " << devices.size() << std::endl;

        auto connection_status = g_client->GetConnectionStatus();
        std::cout << "连接状态: " << connection_status.dump(2) << std::endl;
      }
    }

    // 停止客户端
    std::cout << "正在停止设备客户端..." << std::endl;
    g_client->Stop();
    g_client->Cleanup();

    std::cout << "设备客户端已停止" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "客户端运行异常: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
