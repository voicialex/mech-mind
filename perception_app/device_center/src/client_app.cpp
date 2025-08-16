#include "DeviceClient.hpp"
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

// 统一的设备事件处理器
class UnifiedDeviceEventHandler : public DeviceEventHandler {
 public:
  void OnDeviceConnectionChanged(const std::string& device_id, bool connected) override {
    std::cout << "设备连接状态变化: " << device_id << " " << (connected ? "已连接" : "已断开") << std::endl;
  }

  void OnDeviceStatusChanged(const std::string& device_id, DeviceStatus old_status, DeviceStatus new_status) override {
    std::cout << "设备状态变化: " << device_id << " " 
              << DeviceUtils::StatusToString(old_status) << " -> " 
              << DeviceUtils::StatusToString(new_status) << std::endl;
  }

  void OnDeviceDataReceived(const DeviceData& device_data) override {
    std::cout << "收到设备数据: " << device_data.device_id 
              << " 类型: " << device_data.data_type 
              << " 大小: " << device_data.data_size << std::endl;
  }

  DeviceResponse OnDeviceCommand(const DeviceCommand& command) override {
    std::cout << "处理设备命令: " << command.device_id 
              << " 类型: " << command.command_type << std::endl;

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
    } else if (command.command_type == "status") {
      response.data["status"] = "online";
      response.data["cpu_usage"] = 20.0f;
      response.data["memory_usage"] = 30.0f;
      response.data["temperature"] = 32.0f;
    } else {
      response.success = false;
      response.message = "未知命令类型";
      response.error_code = 1001;
    }

    return response;
  }

  void OnDeviceError(const std::string& device_id, uint16_t error_code, const std::string& error_message) override {
    std::cout << "设备错误: " << device_id 
              << " 错误码: " << error_code 
              << " 消息: " << error_message << std::endl;
  }
};

int main(int argc, char *argv[]) {
  std::cout << "=== Device Center Client (优化版) ===" << std::endl;

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

    // 注册统一的事件处理器
    std::cout << "正在注册事件处理器..." << std::endl;
    auto event_handler = std::make_shared<UnifiedDeviceEventHandler>();
    g_client->RegisterEventHandler(event_handler);

    // 启动客户端
    std::cout << "正在启动设备客户端..." << std::endl;
    if (!g_client->Start()) {
      std::cerr << "客户端启动失败!" << std::endl;
      return -1;
    }

    // 发现服务器（等待一段时间以接收UDP广播）
    std::cout << "正在发现设备服务器..." << std::endl;
    const int discovery_timeout_ms = 5000;
    const int discovery_step_ms = 500;
    int waited_ms = 0;
    std::vector<std::string> servers;
    while (g_running && waited_ms < discovery_timeout_ms) {
      servers = g_client->DiscoverServers("Device Management Server");
      if (!servers.empty()) {
        std::cout << "发现服务器: " << servers[0] << std::endl;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(discovery_step_ms));
      waited_ms += discovery_step_ms;
    }
    if (servers.empty()) {
      std::cout << "在 " << (discovery_timeout_ms/1000) << " 秒内未发现服务器，将继续后台监听并稍后重试连接" << std::endl;
    }

    // 尝试连接到服务器（重试若干次，不因失败直接退出）
    std::cout << "正在连接到设备服务器..." << std::endl;
    const int max_connect_attempts = 5;
    bool connected = false;
    for (int attempt = 1; g_running && attempt <= max_connect_attempts && !connected; ++attempt) {
      std::cout << "尝试连接(" << attempt << "/" << max_connect_attempts << ")..." << std::endl;
      if (g_client->ConnectToServer()) {
        connected = true;
        break;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (!connected) {
      std::cerr << "暂未连接到服务器，客户端将保持运行并继续监听与重试（按 Ctrl+C 退出）" << std::endl;
    }

    if (connected) {
      std::cout << "设备客户端启动成功!" << std::endl;
    } else {
      std::cout << "设备客户端已启动（未连接）。等待发现并自动重试..." << std::endl;
    }
    std::cout << "客户端ID: " << g_client->GetConfig().service_id << std::endl;
    std::cout << "本地端口: " << g_client->GetConfig().local_port << std::endl;
    std::cout << "按 Ctrl+C 停止客户端" << std::endl;

    // 注册示例设备
    std::cout << "正在注册示例设备..." << std::endl;

    // 注册相机设备
    nlohmann::json camera_config;
    camera_config["resolution"] = "1920x1080";
    camera_config["fps"] = 30;
    camera_config["exposure"] = "auto";

    if (g_client->RegisterDevice("camera_001", camera_config)) {
      std::cout << "相机设备注册成功" << std::endl;
    } else {
      std::cerr << "相机设备注册失败" << std::endl;
    }

    // 注册传感器设备
    nlohmann::json sensor_config;
    sensor_config["type"] = "temperature";
    sensor_config["unit"] = "celsius";
    sensor_config["precision"] = 0.1;

    if (g_client->RegisterDevice("sensor_001", sensor_config)) {
      std::cout << "传感器设备注册成功" << std::endl;
    } else {
      std::cerr << "传感器设备注册失败" << std::endl;
    }

    // 运行客户端循环
    while (g_running) {
      std::this_thread::sleep_for(std::chrono::seconds(1));

      // 定期输出统计信息
      static int counter = 0;
      if (++counter % 30 == 0) {  // 每30秒输出一次统计信息
        auto stats = g_client->GetClientStatistics();
        std::cout << "客户端统计: " << stats.dump(2) << std::endl;

        auto devices = g_client->GetAllDevices();
        std::cout << "已注册设备数量: " << devices.size() << std::endl;

        auto online_devices = g_client->GetOnlineDevices();
        std::cout << "在线设备数量: " << online_devices.size() << std::endl;

        // 上报设备状态
        for (const auto& device : devices) {
          DeviceStatusInfo status;
          status.device_id = device.device_id;
          status.status = DeviceStatus::Online;
          status.status_message = "正常运行";
          status.timestamp = DeviceUtils::GetCurrentTimestamp();
          status.cpu_usage = 20.0f + (rand() % 20);  // 模拟CPU使用率
          status.memory_usage = 30.0f + (rand() % 30);  // 模拟内存使用率
          status.temperature = 30.0f + (rand() % 10);  // 模拟温度
          status.status_data["uptime"] = counter * 30;
          status.status_data["data_count"] = rand() % 1000;

          g_client->ReportDeviceStatus(device.device_id, status);
        }

        // 模拟设备数据上报
        for (const auto& device : devices) {
          DeviceData data;
          data.device_id = device.device_id;
          data.data_type = "sensor_data";
          data.timestamp = DeviceUtils::GetCurrentTimestamp();
          data.data["value"] = rand() % 100;
          data.data["unit"] = "raw";
          data.data_size = 16;

          g_client->ReportDeviceData(data);
        }
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
