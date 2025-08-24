#include "EndpointServer.hpp"
#include "communication/endpoints/services/ServiceInspector.hpp"
#include "communication/endpoints/services/EndpointService.hpp"
#include "communication/transports/AsioTransport.hpp"
#include "runtime/configure/ConfigHelper.hpp"
#include "message/IMessageProtocol.hpp"
#include "Logger.hpp"
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace perception;

EndpointServer::EndpointServer(const EndpointIdentity &config) : EndpointService(config) {}

EndpointServer::~EndpointServer() {
  Stop();
  Cleanup();
}

bool EndpointServer::Initialize() {
  if (!EndpointService::Initialize()) {
    return false;
  }

  // 从配置文件中读取心跳设置
  auto &heartbeat_config = ConfigHelper::getInstance().communication_config_.heartbeat;
  EnableHeartbeat(heartbeat_config.enable);

  return true;
}

bool EndpointServer::Start() {
  if (!EndpointService::Start()) {
    return false;
  }

  // 启动心跳监控
  if (heartbeat_enabled_.load()) {
    StartHeartbeatMonitor();
  }

  return true;
}

void EndpointServer::Stop() {
  std::cout << "正在停止服务器..." << std::endl;

  // 停止心跳监控
  StopHeartbeatMonitor();

  EndpointService::Stop();
  std::cout << "服务器已停止" << std::endl;
}

void EndpointServer::Cleanup() {
  // 注销心跳回调函数
  UnregisterHeartbeatCallbacks();

  EndpointService::Cleanup();

  // 清理所有客户端
  std::lock_guard<std::mutex> lock(clients_mutex_);
  clients_.clear();
}

bool EndpointServer::RegisterClient(const std::string &client_id, const EndpointIdentity &client_info) {
  std::lock_guard<std::mutex> lock(clients_mutex_);

  // 获取配置参数
  auto &config = ConfigHelper::getInstance().communication_config_.server;
  if (clients_.size() >= config.max_clients) {
    return false;
  }

  clients_[client_id] = client_info;
  statistics_.total_clients_registered++;

  return true;
}

bool EndpointServer::UnregisterClient(const std::string &client_id) {
  std::lock_guard<std::mutex> lock(clients_mutex_);

  auto it = clients_.find(client_id);
  if (it != clients_.end()) {
    clients_.erase(it);
    return true;
  }

  return false;
}

bool EndpointServer::DisconnectClient(const std::string &client_id) {
  // 物理断开
  if (GetTransport()) {
    GetTransport()->Disconnect(client_id);
  }
  return true;
}

EndpointIdentity EndpointServer::GetClientInfo(const std::string &client_id) const {
  std::lock_guard<std::mutex> lock(clients_mutex_);

  auto it = clients_.find(client_id);
  return it != clients_.end() ? it->second : EndpointIdentity{};
}

std::vector<EndpointIdentity> EndpointServer::GetAllClients() const {
  std::lock_guard<std::mutex> lock(clients_mutex_);

  std::vector<EndpointIdentity> result;
  for (const auto &[id, info] : clients_) {
    result.push_back(info);
  }

  return result;
}

std::vector<EndpointIdentity> EndpointServer::GetOnlineClients() const {
  std::lock_guard<std::mutex> lock(clients_mutex_);

  std::vector<EndpointIdentity> result;
  for (const auto &[id, info] : clients_) {
    // 使用连接状态判断是否在线
    if (GetTransport() && GetTransport()->IsConnected(id)) {
      result.push_back(info);
    }
  }

  return result;
}

bool EndpointServer::HasClient(const std::string &client_id) const {
  std::lock_guard<std::mutex> lock(clients_mutex_);
  return clients_.find(client_id) != clients_.end();
}

bool EndpointServer::IsClientOnline(const std::string &client_id) const {
  std::lock_guard<std::mutex> lock(clients_mutex_);

  auto it = clients_.find(client_id);
  if (it == clients_.end()) {
    return false;
  }

  // 使用连接状态判断是否在线
  return GetTransport() && GetTransport()->IsConnected(client_id);
}

bool EndpointServer::SendToClient(const std::string &client_id, const std::vector<uint8_t> &message_data) {
  LOG_DEBUG_STREAM << "[TX] 服务器发送 -> client_id=" << client_id << ", size=" << message_data.size() << " bytes";
  if (EndpointService::SendMessage(client_id, message_data)) {
    LOG_DEBUG_STREAM << "[TX] 服务器发送成功 -> client_id=" << client_id;
    return true;
  }
  LOG_WARNING_STREAM << "[TX] 服务器发送失败 -> client_id=" << client_id;

  return false;
}

std::vector<uint8_t> EndpointServer::SendRequestToClient(const std::string &client_id,
                                                         const std::vector<uint8_t> &request_data,
                                                         uint32_t timeout_ms) {
  // 简化实现：直接发送消息，不等待响应
  if (SendToClient(client_id, request_data)) {
    return {};  // 返回空响应
  }
  return {};
}

void EndpointServer::BroadcastToClients(const std::vector<uint8_t> &message_data,
                                        const std::vector<std::string> &client_ids) {
  if (client_ids.empty()) {
    // 广播给所有在线客户端
    auto online_clients = GetOnlineClients();
    for (const auto &client : online_clients) {
      EndpointService::SendMessage(client.id, message_data);
    }
  } else {
    // 广播给指定客户端
    for (const auto &client_id : client_ids) {
      if (IsClientOnline(client_id)) {
        EndpointService::SendMessage(client_id, message_data);
      }
    }
  }

  statistics_.total_broadcasts++;
}

void EndpointServer::RegisterEventHandler(EventHandler::Ptr handler) {
  user_handler_ = handler;
  // 用内部处理器封装用户处理器，实现服务端内部状态维护
  EndpointService::RegisterEventHandler(std::make_shared<InternalServerEventHandler>(this));
}

bool EndpointServer::SendMessage(const std::string &target_id, const std::vector<uint8_t> &message_data,
                                 uint32_t timeout_ms) {
  if (timeout_ms > 0) {
    return SendRequestToClient(target_id, message_data, timeout_ms).size() > 0;
  } else {
    return SendToClient(target_id, message_data);
  }
}

void EndpointServer::BroadcastMessage(const std::vector<uint8_t> &message_data, const std::string &target_name) {
  BroadcastToClients(message_data, {});
}

// 私有方法实现

void EndpointServer::CleanupOfflineClients() {
  uint64_t current_time = GetCurrentTimestamp();
  std::vector<std::string> offline_clients;

  {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (const auto &[id, info] : clients_) {
      // 使用连接状态和活动时间判断是否离线
      if (!GetTransport() || !GetTransport()->IsConnected(id)) {
        // 获取配置参数
        auto &config = ConfigHelper::getInstance().communication_config_.server;
        if ((current_time - info.last_activity) > config.client_timeout) {
          offline_clients.push_back(id);
        }
      }
    }
  }

  for (const auto &client_id : offline_clients) {
    UnregisterClient(client_id);
  }
}

uint64_t EndpointServer::GetCurrentTimestamp() const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

// 内部事件处理器：先维护，再转发
void EndpointServer::InternalServerEventHandler::OnMessageReceived(const std::string &endpoint_id,
                                                                   const std::vector<uint8_t> &message_data) {
  {
    std::lock_guard<std::mutex> lock(server_->clients_mutex_);
    auto it = server_->clients_.find(endpoint_id);
    if (it != server_->clients_.end()) {
      it->second.last_activity = server_->GetCurrentTimestamp();
      it->second.activity_count++;
    }
  }

  // 所有消息（包括心跳消息）都由基类的 MessageRouter 统一处理
  // 这里只记录非心跳消息的日志，避免日志噪音
  LOG_DEBUG_STREAM << "[RX] 服务器收到消息 <- client_id=" << endpoint_id << ", size=" << message_data.size();

  // 转发给用户处理器
  if (server_->user_handler_) server_->user_handler_->OnMessageReceived(endpoint_id, message_data);
}

void EndpointServer::InternalServerEventHandler::OnConnectionChanged(const std::string &endpoint_id, bool connected,
                                                                     const ConnectionInfo &connection_info) {
  bool previous_connected = false;
  {
    std::lock_guard<std::mutex> lock(server_->clients_mutex_);
    auto &info = server_->clients_[endpoint_id];
    previous_connected = server_->GetTransport() && server_->GetTransport()->IsConnected(endpoint_id);
    info = connection_info.remote_endpoint;  // 覆盖为远端身份
    info.last_activity = server_->GetCurrentTimestamp();
    if (connected && !previous_connected) {
      server_->statistics_.total_clients_connected++;
    } else if (!connected && previous_connected) {
      if (server_->statistics_.total_clients_connected > 0) server_->statistics_.total_clients_connected--;
    }
  }
  if (server_->user_handler_) server_->user_handler_->OnConnectionChanged(endpoint_id, connected, connection_info);
}

void EndpointServer::InternalServerEventHandler::OnError(const std::string &endpoint_id, uint16_t error_code,
                                                         const std::string &error_message) {
  {
    std::lock_guard<std::mutex> lock(server_->clients_mutex_);
    auto it = server_->clients_.find(endpoint_id);
    if (it != server_->clients_.end()) {
      it->second.last_activity = server_->GetCurrentTimestamp();
    }
  }
  if (server_->user_handler_) server_->user_handler_->OnError(endpoint_id, error_code, error_message);
}

// 心跳支持实现
void EndpointServer::EnableHeartbeat(bool enable) {
  heartbeat_enabled_ = enable;
  LOG_INFO_STREAM << "[HB] 服务器心跳功能 " << (enable ? "已启用" : "已禁用") << " - ID: " << config_.id;
}

bool EndpointServer::IsHeartbeatEnabled() const { return heartbeat_enabled_.load(); }

void EndpointServer::OnHeartbeatRequest(std::shared_ptr<ITransport> transport, const std::string &endpoint_id,
                                        uint16_t message_id, uint8_t sub_message_id,
                                        const std::vector<uint8_t> &payload) {
  if (!heartbeat_enabled_.load()) return;

  // 服务器收到心跳请求，立即回复
  LOG_DEBUG_STREAM << "[HB] 服务器收到心跳请求 <- client_id=" << endpoint_id;
  auto response_data = MessageFactory::CreateHeartbeatResponseMessage();
  if (transport) {
    transport->SendMessage(endpoint_id, response_data);
    LOG_DEBUG_STREAM << "[HB] 服务器发送心跳响应 -> client_id=" << endpoint_id;
  }

  // 更新客户端心跳信息
  uint64_t current_time = GetCurrentTimestamp();
  {
    std::lock_guard<std::mutex> lock(heartbeat_info_mutex_);
    auto &heartbeat_info = client_heartbeat_info_[endpoint_id];
    heartbeat_info.last_request_time = current_time;
    heartbeat_info.total_requests++;
  }
}

void EndpointServer::OnHeartbeatResponse(std::shared_ptr<ITransport> transport, const std::string &endpoint_id,
                                         uint16_t message_id, uint8_t sub_message_id,
                                         const std::vector<uint8_t> &payload) {
  if (!heartbeat_enabled_.load()) return;

  // 服务器收到心跳响应，更新客户端心跳信息
  uint64_t current_time = GetCurrentTimestamp();
  LOG_DEBUG_STREAM << "[HB] 服务器收到心跳响应 <- client_id=" << endpoint_id;

  // 直接更新心跳信息，不再调用 UpdateClientHeartbeatResponse
  {
    std::lock_guard<std::mutex> lock(heartbeat_info_mutex_);
    auto &heartbeat_info = client_heartbeat_info_[endpoint_id];
    heartbeat_info.last_response_time = current_time;
    heartbeat_info.consecutive_missed = 0;
    heartbeat_info.total_responses++;
    heartbeat_info.is_alive = true;

    LOG_DEBUG_STREAM << "[HB] 更新客户端心跳响应 <- client_id=" << endpoint_id
                     << ", total_responses=" << heartbeat_info.total_responses << ", response_rate=" << std::fixed
                     << std::setprecision(1) << (heartbeat_info.GetResponseRate() * 100) << "%";
  }
}

void EndpointServer::StartHeartbeatMonitor() {
  if (!heartbeat_enabled_.load() || heartbeat_monitor_running_.load()) return;

  heartbeat_monitor_running_.store(true);
  LOG_INFO_STREAM << "[HB] 服务器心跳监控启动中...";

  heartbeat_monitor_thread_ = std::thread([this]() {
    auto &heartbeat_config = ConfigHelper::getInstance().communication_config_.heartbeat;
    LOG_INFO_STREAM << "[HB] 服务器心跳监控线程已启动 - interval=" << heartbeat_config.interval
                    << "ms, timeout_multiplier=" << heartbeat_config.timeout_multiplier
                    << ", max_missed=" << heartbeat_config.max_missed_responses;

    uint64_t last_check_time = GetCurrentTimestamp();

    while (heartbeat_monitor_running_.load()) {
      try {
        uint64_t current_time = GetCurrentTimestamp();
        auto connected_endpoints = GetConnectedEndpoints();
        std::vector<std::string> clients_to_disconnect;

        // 分离发送请求和检查超时的逻辑
        {
          std::lock_guard<std::mutex> lock(heartbeat_info_mutex_);
          // 1. 发送心跳请求
          for (const auto &endpoint_id : connected_endpoints) {
            auto &heartbeat_info = client_heartbeat_info_[endpoint_id];
            // 检查是否需要发送心跳请求
            if (current_time - heartbeat_info.last_request_time >= heartbeat_config.interval) {
              SendHeartbeatRequest(endpoint_id);
              heartbeat_info.last_request_time = current_time;
              heartbeat_info.total_requests++;
              LOG_INFO_STREAM << "[HB] 发送心跳请求 -> client_id=" << endpoint_id
                              << ", total_requests=" << heartbeat_info.total_requests;
            }
          }

          // 2. 检查心跳超时（基于最后一次响应时间）
          uint64_t timeout_threshold = heartbeat_config.interval * heartbeat_config.timeout_multiplier;

          for (const auto &endpoint_id : connected_endpoints) {
            auto &heartbeat_info = client_heartbeat_info_[endpoint_id];

            // 只有当已经发送过请求且收到过响应时才检查超时
            if (heartbeat_info.last_request_time > 0 && heartbeat_info.last_response_time > 0) {
              uint64_t time_since_last_response = current_time - heartbeat_info.last_response_time;

              if (time_since_last_response > timeout_threshold) {
                heartbeat_info.consecutive_missed++;
                LOG_WARNING_STREAM << "[HB] 客户端心跳响应超时 -> client_id=" << endpoint_id
                                   << ", time_since_response=" << time_since_last_response << "ms"
                                   << ", consecutive_missed=" << heartbeat_info.consecutive_missed
                                   << ", threshold=" << timeout_threshold << "ms";

                if (heartbeat_info.consecutive_missed >= heartbeat_config.max_missed_responses) {
                  heartbeat_info.is_alive = false;
                  clients_to_disconnect.push_back(endpoint_id);
                  LOG_ERROR_STREAM << "[HB] 客户端心跳超时，准备断开连接 -> client_id=" << endpoint_id
                                   << ", missed_responses=" << heartbeat_info.consecutive_missed;
                }
              } else {
                // 重置连续丢失计数（如果响应时间在正常范围内）
                heartbeat_info.consecutive_missed = 0;
              }
            }
          }

          // 3. 清理断开的客户端心跳信息
          for (auto it = client_heartbeat_info_.begin(); it != client_heartbeat_info_.end();) {
            if (std::find(connected_endpoints.begin(), connected_endpoints.end(), it->first) ==
                connected_endpoints.end()) {
              LOG_DEBUG_STREAM << "[HB] 清理断开客户端的心跳信息 -> client_id=" << it->first;
              it = client_heartbeat_info_.erase(it);
            } else {
              ++it;
            }
          }
        }

        // 4. 断开超时客户端（在锁外执行，避免死锁）
        for (const auto &client_id : clients_to_disconnect) {
          LOG_INFO_STREAM << "[HB] 断开心跳超时客户端 -> client_id=" << client_id;
          if (GetTransport()) {
            GetTransport()->Disconnect(client_id);
          }
        }

        // 使用较短的睡眠间隔，提高响应性
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

      } catch (const std::exception &e) {
        LOG_ERROR_STREAM << "[HB] 心跳监控异常: " << e.what();
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
      }
    }

    LOG_INFO_STREAM << "[HB] 服务器心跳监控线程已停止";
  });
}

void EndpointServer::StopHeartbeatMonitor() {
  if (!heartbeat_monitor_running_.load()) return;

  LOG_INFO_STREAM << "[HB] 正在停止服务器心跳监控...";
  heartbeat_monitor_running_.store(false);
  if (heartbeat_monitor_thread_.joinable()) {
    heartbeat_monitor_thread_.join();
  }
  LOG_INFO_STREAM << "[HB] 服务器心跳监控已停止";
}

void EndpointServer::SendHeartbeatRequest(const std::string &target_id) {
  if (!heartbeat_enabled_.load() || !GetTransport()) return;

  auto message_data = MessageFactory::CreateHeartbeatRequestMessage();
  GetTransport()->SendMessage(target_id, message_data);
}

const EndpointServer::ClientHeartbeatInfo *EndpointServer::GetClientHeartbeatInfo(const std::string &client_id) const {
  std::lock_guard<std::mutex> lock(heartbeat_info_mutex_);
  auto it = client_heartbeat_info_.find(client_id);
  return it != client_heartbeat_info_.end() ? &it->second : nullptr;
}

std::unordered_map<std::string, EndpointServer::ClientHeartbeatInfo> EndpointServer::GetAllClientHeartbeatInfo() const {
  std::lock_guard<std::mutex> lock(heartbeat_info_mutex_);
  return client_heartbeat_info_;
}

bool EndpointServer::IsClientHeartbeatAlive(const std::string &client_id) const {
  return ServiceInspector::IsClientHeartbeatAlive(*this, client_id);
}

std::string EndpointServer::GetHeartbeatStatistics() const { return ServiceInspector::GetHeartbeatStatistics(*this); }
