#include "../include/DeviceServer.hpp"
#include <iostream>
#include <signal.h>
#include <csignal>
#include <memory>

using namespace device_center;

// 全局变量用于信号处理
std::unique_ptr<DeviceServer> g_server;
std::atomic<bool> g_running{true};

// 信号处理函数
void SignalHandler(int signal) {
  std::cout << "\n收到信号 " << signal << "，正在关闭服务器..." << std::endl;
  g_running = false;

  if (g_server) {
    g_server->Stop();
  }
}

// 示例设备处理器
class ExampleDeviceHandler : public DeviceHandler {
 public:
  DeviceResponse HandleCommand(const DeviceCommand &command) override {
    std::cout << "处理设备命令: " << command.device_id << " - " << command.command_type << std::endl;

    DeviceResponse response;
    response.device_id = command.device_id;
    response.command_id = command.command_id;
    response.success = true;
    response.message = "命令执行成功";
    response.timestamp = DeviceUtils::GetCurrentTimestamp();
    response.error_code = 0;

    // 根据命令类型处理
    if (command.command_type == "start") {
      response.data["status"] = "started";
    } else if (command.command_type == "stop") {
      response.data["status"] = "stopped";
    } else if (command.command_type == "configure") {
      response.data["config"] = command.parameters;
    }

    return response;
  }

  DeviceStatusInfo GetDeviceStatus(const std::string &device_id) override {
    DeviceStatusInfo status;
    status.device_id = device_id;
    status.status = DeviceStatus::Online;
    status.status_message = "设备正常运行";
    status.timestamp = DeviceUtils::GetCurrentTimestamp();
    status.cpu_usage = 25.5f;
    status.memory_usage = 45.2f;
    status.temperature = 35.0f;
    status.status_data["uptime"] = 3600;
    status.status_data["frame_count"] = 1000;

    return status;
  }

  DeviceInfo GetDeviceInfo(const std::string &device_id) override {
    DeviceInfo info;
    info.device_id = device_id;
    info.device_name = "示例设备";
    info.device_type = DeviceType::Camera;
    info.device_model = "Example-Camera-001";
    info.device_version = "1.0.0";
    info.device_serial = "SN123456789";
    info.device_ip = "192.168.1.100";
    info.device_port = 8080;
    info.capabilities = {DeviceCapability::Capture, DeviceCapability::Configure, DeviceCapability::Status};
    info.status = DeviceStatus::Online;
    info.status_message = "设备在线";
    info.last_heartbeat = DeviceUtils::GetCurrentTimestamp();
    info.config["resolution"] = "1920x1080";
    info.config["fps"] = 30;

    return info;
  }

  std::vector<DeviceType> GetSupportedDeviceTypes() const override { return {DeviceType::Camera, DeviceType::Sensor}; }
};

// 示例事件处理器
class ExampleEventHandler : public DeviceEventHandler {
 public:
  void OnDeviceStatusChanged(const std::string &device_id, DeviceStatus old_status, DeviceStatus new_status) override {
    std::cout << "设备状态变化: " << device_id << " " << DeviceUtils::StatusToString(old_status) << " -> " << DeviceUtils::StatusToString(new_status) << std::endl;
  }

  void OnDeviceDataReceived(const DeviceData &device_data) override {
    std::cout << "收到设备数据: " << device_data.device_id << " 类型: " << device_data.data_type << " 大小: " << device_data.data_size << std::endl;
  }

  void OnDeviceError(const std::string &device_id, uint16_t error_code, const std::string &error_message) override {
    std::cout << "设备错误: " << device_id << " 错误码: " << error_code << " 消息: " << error_message << std::endl;
  }
};

int main(int argc, char *argv[]) {
  std::cout << "=== Device Center Server ===" << std::endl;

  // 设置信号处理
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  // 配置文件路径
  std::string config_path = "config/server_config.json";
  if (argc > 1) {
    config_path = argv[1];
  }

  try {
    // 创建设备服务器
    g_server = std::make_unique<DeviceServer>();

    // 初始化服务器
    std::cout << "正在初始化设备服务器..." << std::endl;
    if (!g_server->Initialize(config_path)) {
      std::cerr << "服务器初始化失败!" << std::endl;
      return -1;
    }

    // 注册设备处理器
    std::cout << "正在注册设备处理器..." << std::endl;
    auto device_handler = std::make_shared<ExampleDeviceHandler>();
    g_server->RegisterDeviceHandler(DeviceType::Camera, device_handler);
    g_server->RegisterDeviceHandler(DeviceType::Sensor, device_handler);

    // 注册事件处理器
    auto event_handler = std::make_shared<ExampleEventHandler>();
    g_server->RegisterEventHandler(event_handler);

    // 启动服务器
    std::cout << "正在启动设备服务器..." << std::endl;
    if (!g_server->Start()) {
      std::cerr << "服务器启动失败!" << std::endl;
      return -1;
    }

    std::cout << "设备服务器启动成功!" << std::endl;
    std::cout << "服务器ID: " << g_server->GetConfig().service_id << std::endl;
    std::cout << "监听端口: " << g_server->GetConfig().local_port << std::endl;
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;

    // 运行服务器循环
    while (g_running) {
      std::this_thread::sleep_for(std::chrono::seconds(1));

      // 定期输出统计信息
      static int counter = 0;
      if (++counter % 30 == 0) {  // 每30秒输出一次统计信息
        auto stats = g_server->GetServerStatistics();
        std::cout << "服务器统计: " << stats.dump(2) << std::endl;

        auto devices = g_server->GetAllDevices();
        std::cout << "已注册设备数量: " << devices.size() << std::endl;

        auto online_devices = g_server->GetOnlineDevices();
        std::cout << "在线设备数量: " << online_devices.size() << std::endl;
      }
    }

    // 停止服务器
    std::cout << "正在停止设备服务器..." << std::endl;
    g_server->Stop();
    g_server->Cleanup();

    std::cout << "设备服务器已停止" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "服务器运行异常: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
