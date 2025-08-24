#include "master_node/MasterNode.hpp"
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

std::unique_ptr<MasterNode> g_master_node = nullptr;
std::atomic<bool> g_running{true};

// 控制数据收发测试的开关
const bool ENABLE_DATA_TEST = false;  // 设置为 false 关闭数据收发测试，专注于心跳日志

void SignalHandler(int signal) {
  std::cout << "\n收到信号 " << signal << "，正在停止Master节点..." << std::endl;
  g_running = false;
  if (g_master_node) {
    g_master_node->Stop();
  }
  exit(0);
}

// 消息处理回调函数
void OnDeviceStatusRequest(std::shared_ptr<ITransport> transport, uint16_t message_id, uint8_t sub_message_id,
                           const std::vector<uint8_t> &payload) {
  LOG_INFO_STREAM << "[MSG] 收到设备状态请求 - 消息ID: 0x" << std::hex << message_id << std::dec;

  if (!payload.empty()) {
    std::string payload_str(payload.begin(), payload.end());
    LOG_INFO_STREAM << "[MSG] 设备状态请求内容: " << payload_str;

    // 处理端口信息消息
    if (payload_str.find("CLIENT_PORT_INFO:") == 0) {
      std::string client_port = payload_str.substr(17);  // 跳过 "CLIENT_PORT_INFO:"
      LOG_INFO_STREAM << "[MSG] 客户端端口信息: " << client_port;

      // 构建回复消息（使用便捷的消息创建函数）
      std::string reply_payload = "SERVER_REPLY:9090_ACK_" + client_port;
      std::vector<uint8_t> serialized_reply =
          MessageFactory::CreateResponseMessage(MessageIds::DEVICE_STATUS, SubMessageIds::READY, reply_payload);

      // 通过传输层发送回复
      if (transport) {
        transport->SendMessage("", serialized_reply);
        LOG_INFO_STREAM << "[MSG] 发送协议回复成功: " << reply_payload;
      }
    }
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

void OnHeartbeatRequest(std::shared_ptr<ITransport> transport, uint16_t message_id, uint8_t sub_message_id,
                        const std::vector<uint8_t> &payload) {
  LOG_DEBUG_STREAM << "[HEARTBEAT] 收到心跳请求 - 消息ID: 0x" << std::hex << message_id << std::dec;

  // 发送心跳响应
  std::vector<uint8_t> heartbeat_response =
      MessageFactory::CreateResponseMessage(MessageIds::HEARTBEAT_RESPONSE, SubMessageIds::IDLE, "");

  if (transport) {
    transport->SendMessage("", heartbeat_response);
    LOG_DEBUG_STREAM << "[HEARTBEAT] 发送心跳响应";
  }
}

int main(int argc, char *argv[]) {
  // 设置信号处理
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  // 设置日志级别
  Logger::getInstance().setLevel(Logger::Level::INFO);

  std::cout << "=== Master节点示例 ===" << std::endl;

  // 加载主配置（单入口）
  auto &config_helper = ConfigHelper::getInstance();
  if (!config_helper.loadConfigFromJson("../../perception_app/config/config.json")) {
    std::cerr << "加载主配置失败，使用默认配置" << std::endl;
  }

  // 打印通信配置
  config_helper.printCommunicationConfig();

  // 创建Master节点配置
  MasterNode::Config config;

  // 从ConfigHelper加载服务发现配置
  config.discovery_address = config_helper.communication_config_.service_discovery.local_address;
  config.discovery_port = config_helper.communication_config_.service_discovery.discovery_port;
  config.broadcast_interval = config_helper.communication_config_.service_discovery.broadcast_interval;

  // 从ConfigHelper加载服务器配置
  config.device_server_config.id = config_helper.communication_config_.server.id;
  config.device_server_config.name = config_helper.communication_config_.server.name;
  config.device_server_config.address = config_helper.communication_config_.server.address;
  config.device_server_config.port = config_helper.communication_config_.server.port;
  config.device_server_config.type = EndpointType::Server;
  // 服务器配置使用硬编码默认值

  // 从ConfigHelper加载Master节点监控配置
  config.client_timeout_interval = config_helper.communication_config_.master_node.client_timeout_interval;
  config.status_check_interval = config_helper.communication_config_.master_node.status_check_interval;
  config.state_sync_interval = config_helper.communication_config_.master_node.state_sync_interval;
  config.enable_auto_cleanup = config_helper.communication_config_.master_node.enable_auto_cleanup;
  // 心跳配置现在统一在 heartbeat 配置节中
  // config.enable_heartbeat = config_helper.communication_config_.heartbeat.enable;

  std::cout << "客户端监控配置:" << std::endl;
  std::cout << "  超时时间: " << config.client_timeout_interval << "ms" << std::endl;
  std::cout << "  状态检查间隔: " << config.status_check_interval << "ms" << std::endl;
  std::cout << "  自动清理: " << (config.enable_auto_cleanup ? "启用" : "禁用") << std::endl;

  // 创建Master节点
  g_master_node = std::make_unique<MasterNode>(config);

  // 初始化并启动Master节点
  if (!g_master_node->Initialize()) {
    std::cerr << "Master节点初始化失败!" << std::endl;
    return 1;
  }

  // 注册消息回调函数（使用新的 MessageRouter 机制）
  g_master_node->RegisterMessageCallback(MessageType::Request, MessageIds::DEVICE_STATUS, SubMessageIds::READY,
                                         OnDeviceStatusRequest);
  g_master_node->RegisterMessageCallback(MessageType::Request, MessageIds::DEVICE_CONTROL, SubMessageIds::IDLE,
                                         OnSystemCommand);
  g_master_node->RegisterMessageCallback(MessageType::Notify, MessageIds::SERVICE_DISCOVERY, SubMessageIds::IDLE,
                                         OnDataTransfer);
  g_master_node->RegisterMessageCallback(MessageType::Request, MessageIds::HEARTBEAT_REQUEST, SubMessageIds::IDLE,
                                         OnHeartbeatRequest);

  if (!g_master_node->Start()) {
    std::cerr << "Master节点启动失败!" << std::endl;
    return 1;
  }

  std::cout << "Master节点启动成功!" << std::endl;
  std::cout << "数据测试: " << (ENABLE_DATA_TEST ? "启用" : "禁用") << std::endl;
  std::cout << "按 Ctrl+C 停止..." << std::endl;

  // 主循环
  try {
    int loop_count = 0;
    while (g_running) {
      loop_count++;
      // 等待5秒
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  } catch (const std::exception &e) {
    std::cerr << "运行时错误: " << e.what() << std::endl;
  }

  // 停止Master节点
  g_master_node->Stop();
  std::cout << "Master节点已停止" << std::endl;

  return 0;
}
