#include "client_node/ClientNode.hpp"
#include "message/IMessageProtocol.hpp"
#include "message/ProtocolDefinitions.hpp"
#include "Logger.hpp"
#include "configure/ConfigHelper.hpp"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace perception;

std::unique_ptr<ClientNode> g_client_node = nullptr;
std::atomic<bool> g_running{true};
std::atomic<bool> g_connected{false};

// 控制数据收发测试的开关
const bool ENABLE_DATA_TEST = false;  // 设置为 false 关闭数据收发测试，专注于心跳日志

void SignalHandler(int signal) {
  std::cout << "\n收到信号 " << signal << "，正在停止Client节点..." << std::endl;
  g_running = false;
  if (g_client_node) {
    g_client_node->Stop();
  }
  exit(0);
}

// 消息处理回调函数
void OnDeviceStatusResponse(std::shared_ptr<ITransport> transport, uint16_t message_id, uint8_t sub_message_id,
                            const std::vector<uint8_t> &payload) {
  LOG_INFO_STREAM << "[MSG] 收到设备状态响应 - 消息ID: 0x" << std::hex << message_id << std::dec;

  if (!payload.empty()) {
    std::string payload_str(payload.begin(), payload.end());
    LOG_INFO_STREAM << "[MSG] 设备状态响应内容: " << payload_str;
  }
}

void OnSystemCommand(std::shared_ptr<ITransport> transport, uint16_t message_id, uint8_t sub_message_id,
                     const std::vector<uint8_t> &payload) {
  LOG_INFO_STREAM << "[MSG] 收到系统命令 - 消息ID: 0x" << std::hex << message_id
                  << ", 子消息ID: " << static_cast<int>(sub_message_id) << std::dec;

  if (!payload.empty()) {
    std::string payload_str(payload.begin(), payload.end());
    LOG_INFO_STREAM << "[MSG] 系统命令内容: " << payload_str;
  }
}

void OnDataTransfer(std::shared_ptr<ITransport> transport, uint16_t message_id, uint8_t sub_message_id,
                    const std::vector<uint8_t> &payload) {
  LOG_INFO_STREAM << "[MSG] 收到数据传输 - 消息ID: 0x" << std::hex << message_id
                  << ", 子消息ID: " << static_cast<int>(sub_message_id) << std::dec;

  LOG_INFO_STREAM << "[MSG] 数据传输大小: " << payload.size() << " 字节";
}

void OnHeartbeatResponse(std::shared_ptr<ITransport> transport, uint16_t message_id, uint8_t sub_message_id,
                         const std::vector<uint8_t> &payload) {
  LOG_DEBUG_STREAM << "[HEARTBEAT] 收到心跳响应 - 消息ID: 0x" << std::hex << message_id << std::dec;
}

// 周期性发送端口信息的线程函数
void PortMessageThread() {
  if (!ENABLE_DATA_TEST) {
    LOG_INFO_STREAM << "[MSG] 数据测试已关闭，端口信息发送线程不启动";
    return;
  }

  LOG_INFO_STREAM << "[MSG] 启动端口信息发送线程";

  while (g_running) {
    if (g_connected && g_client_node) {
      try {
        // 构建端口信息消息（使用便捷的消息创建函数）
        std::string port_info = "CLIENT_PORT_INFO:8081";
        std::vector<uint8_t> message_data =
            MessageFactory::CreateNotifyMessage(MessageIds::DEVICE_STATUS, SubMessageIds::READY, port_info);

        // 发送消息到服务端
        if (g_client_node->GetDeviceClient()->SendMessage("", message_data)) {
          LOG_INFO_STREAM << "[MSG] 发送端口信息成功: " << port_info;
        } else {
          LOG_WARNING_STREAM << "[MSG] 发送端口信息失败";
        }
      } catch (const std::exception &e) {
        LOG_ERROR_STREAM << "[MSG] 发送端口信息异常: " << e.what();
      }
    }

    // 等待3秒
    for (int i = 0; i < 30 && g_running; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  LOG_INFO_STREAM << "[MSG] 端口信息发送线程已退出";
}

int main(int argc, char *argv[]) {
  // 设置信号处理
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  // 设置日志级别
  Logger::getInstance().setLevel(Logger::Level::INFO);

  std::cout << "=== Client节点示例 ===" << std::endl;

  // 加载主配置（单入口）
  auto &config_helper = ConfigHelper::getInstance();
  if (!config_helper.loadConfigFromJson("../../perception_app/config/config.json")) {
    std::cerr << "加载主配置失败，使用默认配置" << std::endl;
  }

  // 打印通信配置
  config_helper.printCommunicationConfig();

  // 创建Client节点配置
  ClientNode::Config config;

  // 从ConfigHelper加载服务发现配置
  config.discovery_address = config_helper.communication_config_.service_discovery.local_address;
  config.discovery_port = config_helper.communication_config_.service_discovery.discovery_port;

  // 从ConfigHelper加载客户端配置
  config.device_client_id = config_helper.communication_config_.client.id;
  config.device_client_name = config_helper.communication_config_.client.name;
  config.controller_client_id = "controller_client_001";
  config.controller_client_name = "Controller Client";
  config.enable_controller_client = false;

  // 创建Client节点
  g_client_node = std::make_unique<ClientNode>(config);

  // 初始化并启动Client节点
  if (!g_client_node->Initialize()) {
    std::cerr << "Client节点初始化失败!" << std::endl;
    return 1;
  }

  // 注册消息回调函数（使用新的 MessageRouter 机制）
  g_client_node->RegisterMessageCallback(MessageType::Response, MessageIds::DEVICE_STATUS, SubMessageIds::READY,
                                         OnDeviceStatusResponse);
  g_client_node->RegisterMessageCallback(MessageType::Request, MessageIds::DEVICE_CONTROL, SubMessageIds::IDLE,
                                         OnSystemCommand);
  g_client_node->RegisterMessageCallback(MessageType::Notify, MessageIds::SERVICE_DISCOVERY, SubMessageIds::IDLE,
                                         OnDataTransfer);
  g_client_node->RegisterMessageCallback(MessageType::Response, MessageIds::HEARTBEAT_RESPONSE, SubMessageIds::IDLE,
                                         OnHeartbeatResponse);

  if (!g_client_node->Start()) {
    std::cerr << "Client节点启动失败!" << std::endl;
    return 1;
  }

  std::cout << "Client节点启动成功!" << std::endl;
  std::cout << "正在等待连接Master服务器..." << std::endl;
  std::cout << "数据测试: " << (ENABLE_DATA_TEST ? "启用" : "禁用") << std::endl;
  std::cout << "按 Ctrl+C 停止..." << std::endl;

  // 启动端口信息发送线程（仅在数据测试启用时）
  std::thread port_thread(PortMessageThread);

  // 主循环
  try {
    while (g_running) {
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  } catch (const std::exception &e) {
    std::cerr << "运行时错误: " << e.what() << std::endl;
  }

  // 等待端口信息发送线程结束
  if (port_thread.joinable()) {
    port_thread.join();
  }

  // 停止Client节点
  g_client_node->Stop();
  std::cout << "Client节点已停止" << std::endl;

  return 0;
}
