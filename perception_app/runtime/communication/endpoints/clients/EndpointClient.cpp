#include "EndpointClient.hpp"
#include "Logger.hpp"
#include "message/IMessageProtocol.hpp"
#include "configure/ConfigHelper.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

using namespace perception;

EndpointClient::EndpointClient(const EndpointIdentity &config) : EndpointService(config), connected_(false) {}

EndpointClient::~EndpointClient() {
  // 仅做清理，避免与外部Stop重复打印
  Cleanup();
}

bool EndpointClient::Initialize() {
  if (!EndpointService::Initialize()) {
    return false;
  }

  // 从配置文件中读取心跳设置
  auto &heartbeat_config = ConfigHelper::getInstance().communication_config_.heartbeat;
  EnableHeartbeat(heartbeat_config.enable);

  return true;
}

bool EndpointClient::Start() {
  if (!EndpointService::Start()) {
    return false;
  }

  // 启动连接监控
  StartConnectionMonitor();

  return true;
}

void EndpointClient::Stop() {
  static std::atomic<bool> stopping_once{false};
  bool expected = false;
  if (!stopping_once.compare_exchange_strong(expected, true)) {
    return;  // 已停止过，避免重复打印
  }
  // 降级为DEBUG，避免噪音
  LOG_DEBUG_STREAM << "正在停止客户端...";

  // 停止监控线程
  StopConnectionMonitor();

  EndpointService::Stop();
  connected_ = false;

  LOG_DEBUG_STREAM << "客户端已停止";
}

void EndpointClient::Cleanup() {
  // 确保线程停止，但不打印客户端停止日志
  StopConnectionMonitor();

  // 注销心跳回调函数
  UnregisterHeartbeatCallbacks();

  EndpointService::Cleanup();
}

bool EndpointClient::ConnectToServer(const std::string &server_address, uint16_t server_port) {
  // 更新运行时服务器地址和端口
  target_server_address_ = server_address;
  target_server_port_ = server_port;

  // 生成服务器ID
  connected_server_id_ = server_address + ":" + std::to_string(server_port);

  // 重置重连计数
  ResetReconnectAttempts();

  // 尝试连接（连接状态由事件回调管理）
  if (TryConnectToServer()) {
    LOG_INFO_STREAM << "[HB] 客户端连接请求已发送 -> server_id=" << connected_server_id_;
    return true;
  }

  // 如果连接失败，启动重连机制
  auto &config = ConfigHelper::getInstance().communication_config_.client;
  if (config.enable_auto_reconnect) {
    reconnect_enabled_ = true;
    LOG_DEBUG_STREAM << "连接失败，启动自动重连机制";
  }

  return false;
}

void EndpointClient::DisconnectFromServer() {
  if (!connected_server_id_.empty()) {
    // 使用传输层直接断开连接
    if (GetTransport()) {
      GetTransport()->Disconnect(connected_server_id_);
    }
    connected_ = false;
    reconnect_enabled_ = false;
    statistics_.total_disconnections++;
  }
}

bool EndpointClient::IsConnectedToServer() const {
  return connected_ && !connected_server_id_.empty() && IsEndpointOnline(connected_server_id_);
}

std::string EndpointClient::GetConnectionStatus() const {
  if (!connected_) {
    return "disconnected";
  }

  if (!connected_server_id_.empty()) {
    if (IsEndpointOnline(connected_server_id_)) {
      return "connected";
    } else {
      return "connecting";
    }
  }

  return "unknown";
}

void EndpointClient::RegisterEventHandler(EventHandler::Ptr handler) {
  user_handler_ = handler;
  // 用内部处理器封装用户处理器，实现客户端内部状态维护
  EndpointService::RegisterEventHandler(std::make_shared<InternalClientEventHandler>(this));
}

bool EndpointClient::SendMessage(const std::string &target_id, const std::vector<uint8_t> &message_data,
                                 uint32_t timeout_ms) {
  if (!connected_) {
    return false;
  }

  std::string target = target_id.empty() ? connected_server_id_ : target_id;

  if (EndpointService::SendMessage(target, message_data, timeout_ms)) {
    return true;
  }

  return false;
}

void EndpointClient::BroadcastMessage(const std::vector<uint8_t> &message_data, const std::string &target_name) {
  if (!connected_) {
    return;
  }

  EndpointService::BroadcastMessage(message_data, target_name);
}

// 私有方法实现

void EndpointClient::AutoReconnect() {
  uint64_t current_time = GetCurrentTimestamp();

  // 获取配置参数
  auto &config = ConfigHelper::getInstance().communication_config_.client;

  // 检查重连间隔
  if (current_time - statistics_.last_reconnect_time < config.reconnect_interval) {
    return;
  }

  // 检查是否超过最大重连次数
  if (statistics_.reconnect_attempts >= config.max_reconnect_attempts) {
    reconnect_enabled_ = false;
    LOG_DEBUG_STREAM << "达到最大重连次数，停止重连";
    return;
  }

  statistics_.reconnect_attempts++;
  statistics_.last_reconnect_time = current_time;

  auto &client_config = ConfigHelper::getInstance().communication_config_.client;
  LOG_DEBUG_STREAM << "尝试自动重连到服务器，第 " << statistics_.reconnect_attempts << "/"
                   << client_config.max_reconnect_attempts << " 次尝试";

  // 尝试连接
  TryConnectToServer();
}

void EndpointClient::StartConnectionMonitor() {
  if (monitor_running_.load()) {
    return;
  }

  monitor_running_ = true;

  LOG_INFO_STREAM << "[HB] 客户端监控线程启动中...";
  // 启动连接监控线程
  connection_monitor_thread_ = std::thread([this]() { ConnectionMonitorLoop(); });

  LOG_INFO_STREAM << "[HB] 客户端连接监控线程已启动";
}

void EndpointClient::StopConnectionMonitor() {
  if (!monitor_running_.load()) {
    return;
  }

  LOG_DEBUG_STREAM << "正在停止连接监控线程...";
  monitor_running_ = false;

  // 使用超时机制等待线程退出（缩短为1秒）
  if (connection_monitor_thread_.joinable()) {
    auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(1);
    while (connection_monitor_thread_.joinable() && std::chrono::system_clock::now() < timeout) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (connection_monitor_thread_.joinable()) {
      LOG_DEBUG_STREAM << "强制分离连接监控线程";
      connection_monitor_thread_.detach();
    } else {
      connection_monitor_thread_.join();
    }
  }

  LOG_DEBUG_STREAM << "连接监控线程已停止";
}

void EndpointClient::ConnectionMonitorLoop() {
  while (monitor_running_.load()) {
    try {
      // 检查是否需要重连
      if (reconnect_enabled_.load() && !connected_.load()) {
        AutoReconnect();
      }

      // 检查连接状态
      if (connected_.load() && !connected_server_id_.empty()) {
        if (!IsEndpointOnline(connected_server_id_)) {
          LOG_DEBUG_STREAM << "检测到连接断开，标记为断开状态";
          connected_ = false;

          auto &config = ConfigHelper::getInstance().communication_config_.client;
          if (config.enable_auto_reconnect && !reconnect_enabled_.load()) {
            reconnect_enabled_ = true;
          }
        }
      }

      // 使用更短的检查间隔，提高响应速度
      auto &config = ConfigHelper::getInstance().communication_config_.client;
      for (int i = 0; i < 10 && monitor_running_.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(config.connection_check_interval / 10));
      }
    } catch (const std::exception &e) {
      std::cerr << "连接监控异常: " << e.what() << std::endl;
      // 异常时使用更短的等待时间
      for (int i = 0; i < 10 && monitor_running_.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    }
  }
}

bool EndpointClient::TryConnectToServer() {
  uint64_t current_time = GetCurrentTimestamp();
  last_connection_attempt_.store(current_time);

  LOG_DEBUG_STREAM << "尝试连接到服务器: " << connected_server_id_ << " (" << target_server_address_ << ":"
                   << target_server_port_ << ")";

  // 使用运行时的地址和端口直接连接
  if (transport_ && transport_->Connect(connected_server_id_, target_server_address_, target_server_port_)) {
    LOG_DEBUG_STREAM << "连接成功到服务器: " << connected_server_id_ << " (" << target_server_address_ << ":"
                     << target_server_port_ << ")";
    return true;
  }

  LOG_DEBUG_STREAM << "连接失败到服务器: " << connected_server_id_;
  return false;
}

void EndpointClient::ResetReconnectAttempts() {
  statistics_.reconnect_attempts = 0;
  statistics_.last_reconnect_time = 0;
}

// 心跳支持实现
void EndpointClient::EnableHeartbeat(bool enable) {
  heartbeat_enabled_ = enable;
  LOG_INFO_STREAM << "[HB] 客户端心跳功能 " << (enable ? "已启用" : "已禁用") << " - ID: " << config_.id;
}

bool EndpointClient::IsHeartbeatEnabled() const { return heartbeat_enabled_.load(); }

void EndpointClient::OnHeartbeatRequest(std::shared_ptr<ITransport> transport, const std::string &endpoint_id,
                                        uint16_t message_id, uint8_t sub_message_id,
                                        const std::vector<uint8_t> &payload) {
  if (!heartbeat_enabled_.load()) return;

  // 客户端收到心跳请求，立即回复
  LOG_DEBUG_STREAM << "[HB] 客户端收到心跳请求 <- server_id=" << endpoint_id;
  auto response_data = MessageFactory::CreateHeartbeatResponseMessage();
  if (transport) {
    transport->SendMessage(endpoint_id, response_data);
    LOG_INFO_STREAM << "[HB] 客户端发送心跳响应 -> server_id=" << endpoint_id;
  }
}

void EndpointClient::OnHeartbeatResponse(std::shared_ptr<ITransport> transport, const std::string &endpoint_id,
                                         uint16_t message_id, uint8_t sub_message_id,
                                         const std::vector<uint8_t> &payload) {
  // 客户端通常不需要处理心跳响应
  LOG_DEBUG_STREAM << "[HB] 客户端收到心跳响应 <- server_id=" << endpoint_id;
}

// 内部事件处理器实现
void EndpointClient::InternalClientEventHandler::OnMessageReceived(const std::string &endpoint_id,
                                                                   const std::vector<uint8_t> &message_data) {
  // 所有消息（包括心跳消息）都由基类的 MessageRouter 统一处理
  // 这里只记录非心跳消息的日志，避免日志噪音
  LOG_DEBUG_STREAM << "[RX] 客户端收到消息 <- server_id=" << endpoint_id << ", size=" << message_data.size();

  // 转发给用户处理器
  if (client_->user_handler_) client_->user_handler_->OnMessageReceived(endpoint_id, message_data);
}

void EndpointClient::InternalClientEventHandler::OnConnectionChanged(const std::string &endpoint_id, bool connected,
                                                                     const ConnectionInfo &connection_info) {
  LOG_INFO_STREAM << "[CONN] InternalClientEventHandler::OnConnectionChanged 被调用 -> endpoint_id=" << endpoint_id
                  << ", connected=" << connected << ", expected_server_id=" << client_->connected_server_id_;

  // 更新客户端内部连接状态
  if (endpoint_id == client_->connected_server_id_) {
    client_->connected_ = connected;
    if (connected) {
      LOG_INFO_STREAM << "[CONN] 客户端连接状态更新 -> connected=true, server_id=" << endpoint_id;
    } else {
      LOG_INFO_STREAM << "[CONN] 客户端连接状态更新 -> connected=false, server_id=" << endpoint_id;
    }
  } else {
    LOG_WARNING_STREAM << "[CONN] 客户端连接事件ID不匹配 -> received=" << endpoint_id
                       << ", expected=" << client_->connected_server_id_;
  }
  // 转发给用户处理器
  if (client_->user_handler_) client_->user_handler_->OnConnectionChanged(endpoint_id, connected, connection_info);
}

void EndpointClient::InternalClientEventHandler::OnError(const std::string &endpoint_id, uint16_t error_code,
                                                         const std::string &error_message) {
  // 转发给用户处理器
  if (client_->user_handler_) client_->user_handler_->OnError(endpoint_id, error_code, error_message);
}
