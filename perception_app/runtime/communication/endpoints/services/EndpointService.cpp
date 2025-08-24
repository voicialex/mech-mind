#include "EndpointService.hpp"
#include "communication/transports/AsioTransport.hpp"
#include "message/IMessageProtocol.hpp"
#include "configure/ConfigHelper.hpp"
#include "Logger.hpp"
#include <iostream>
#include <sstream>

using namespace perception;

EndpointService::EndpointService(const EndpointIdentity &config)
    : config_(config), state_(EndpointState::Stopped), initialized_(false) {
  LOG_INFO_STREAM << "EndpointService 创建 - ID: " << config.id;
}

EndpointService::~EndpointService() { Cleanup(); }

bool EndpointService::Initialize() {
  if (initialized_.load()) {
    return true;
  }

  if (state_ != EndpointState::Stopped) {
    return false;
  }

  SetState(EndpointState::Starting);

  try {
    // 初始化各个组件
    InitializeTransport();

    statistics_.start_time = GetCurrentTimestamp();
    initialized_ = true;
    SetState(EndpointState::Stopped);  // 初始化完成，回到停止状态
    return true;
  } catch (const std::exception &e) {
    std::cerr << "端点服务初始化失败: " << e.what() << std::endl;
    SetState(EndpointState::Error);
    return false;
  }
}

bool EndpointService::Start() {
  if (!initialized_.load()) {
    LOG_ERROR_STREAM << "端点服务未初始化，无法启动";
    return false;
  }

  if (state_ != EndpointState::Stopped) {
    LOG_WARNING_STREAM << "端点服务状态不正确，当前状态: " << static_cast<uint32_t>(state_.load());
    return false;
  }

  if (running_.load()) {
    LOG_INFO_STREAM << "端点服务已在运行中";
    return true;
  }

  LOG_INFO_STREAM << "启动端点服务 - 服务ID: " << config_.id
                  << ", 类型: " << (config_.type == EndpointType::Server ? "服务器" : "客户端");

  SetState(EndpointState::Starting);

  try {
    // 初始化传输层
    if (transport_ && !transport_->Initialize()) {
      LOG_ERROR_STREAM << "传输层初始化失败";
      SetState(EndpointState::Error);
      return false;
    }

    // 启动传输层
    if (transport_ && !transport_->Start()) {
      LOG_ERROR_STREAM << "传输层启动失败";
      SetState(EndpointState::Error);
      return false;
    }

    running_ = true;
    SetState(EndpointState::Running);

    LOG_INFO_STREAM << "端点服务启动成功 - 服务ID: " << config_.id;
    return true;
  } catch (const std::exception &e) {
    LOG_ERROR_STREAM << "端点服务启动失败: " << e.what();
    SetState(EndpointState::Error);
    return false;
  }
}

void EndpointService::Stop() {
  if (!running_.load()) {
    return;
  }

  LOG_INFO_STREAM << "正在停止端点服务 - 服务ID: " << config_.id;
  SetState(EndpointState::Stopping);
  running_ = false;

  // 停止传输层
  if (transport_) {
    LOG_INFO_STREAM << "正在停止传输层";
    transport_->Stop();
  }

  SetState(EndpointState::Stopped);
  LOG_INFO_STREAM << "端点服务已停止 - 服务ID: " << config_.id;
}

void EndpointService::Cleanup() {
  Stop();

  // 注销心跳回调函数
  UnregisterHeartbeatCallbacks();

  // 清理连接
  {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    endpoint_connections_.clear();
  }

  // 清理事件处理器
  event_handler_.reset();

  // 重置组件
  transport_.reset();
  message_router_.reset();

  // 重置统计信息
  statistics_.Reset();
  initialized_ = false;
}

bool EndpointService::SendMessage(const std::string &target_id, const std::vector<uint8_t> &message_data,
                                  uint32_t timeout_ms) {
  if (!transport_ || !running_.load()) {
    return false;
  }

  LOG_DEBUG_STREAM << "[TX] 发送消息 -> target_id=" << target_id << ", size=" << message_data.size() << " bytes";

  if (transport_->SendMessage(target_id, message_data)) {
    statistics_.messages_sent++;
    return true;
  }

  statistics_.errors++;
  return false;
}

void EndpointService::BroadcastMessage(const std::vector<uint8_t> &message_data, const std::string &target_name) {
  if (!transport_ || !running_.load()) {
    return;
  }

  transport_->BroadcastMessage(message_data, target_name);
  statistics_.messages_sent++;
}

void EndpointService::RegisterEventHandler(EventHandler::Ptr handler) { event_handler_ = std::move(handler); }

bool EndpointService::IsEndpointOnline(const std::string &endpoint_id) const { return IsConnected(endpoint_id); }

IEndpointService::EndpointState EndpointService::GetState() const { return state_.load(); }

const EndpointIdentity &EndpointService::GetConfig() const { return config_; }

bool EndpointService::IsRunning() const { return running_.load(); }

// 受保护方法实现
void EndpointService::SetState(EndpointState state) { state_.store(state); }

std::string EndpointService::GenerateEndpointId() const {
  return config_.id + "_" + std::to_string(GetCurrentTimestamp());
}

uint64_t EndpointService::GetCurrentTimestamp() const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
      .count();
}

bool EndpointService::IsConnected(const std::string &endpoint_id) const {
  if (!transport_) {
    return false;
  }

  return transport_->IsConnected(endpoint_id);
}

std::vector<std::string> EndpointService::GetConnectedEndpoints() const {
  std::lock_guard<std::mutex> lock(connections_mutex_);
  std::vector<std::string> connected_endpoints;

  for (const auto &[id, connected] : endpoint_connections_) {
    if (connected) {
      connected_endpoints.push_back(id);
    }
  }

  return connected_endpoints;
}

// 私有方法实现
bool EndpointService::InitializeTransport() {
  LOG_INFO_STREAM << "初始化传输层 - 服务ID: " << config_.id
                  << ", 类型: " << (config_.type == EndpointType::Server ? "服务器" : "客户端");

  try {
    transport_ = std::make_shared<AsioTransport>(config_);
    // 创建并注册内部事件处理器
    internal_event_handler_ =
        std::static_pointer_cast<ITransport::EventHandler>(std::make_shared<InternalEventHandler>(this));
    transport_->RegisterEventHandler(internal_event_handler_);
    // 初始化消息路由器
    message_router_ = std::make_shared<MessageRouter>();
    message_router_->InitializeDefaultRoutes();
    // 注册心跳回调函数
    RegisterHeartbeatCallbacks();

    LOG_INFO_STREAM << "传输层初始化完成";
    return true;
  } catch (const std::exception &e) {
    LOG_ERROR_STREAM << "传输层初始化失败: " << e.what();
    return false;
  }
}

// InternalEventHandler 实现
class EndpointService::InternalEventHandler : public ITransport::EventHandler {
 public:
  explicit InternalEventHandler(EndpointService *service) : service_(service) {}

  void OnMessageReceived(const std::string &endpoint_id, const std::vector<uint8_t> &message_data) override {
    if (!service_) return;

    try {
      LOG_DEBUG_STREAM << "[RX] 收到消息 -> endpoint_id=" << endpoint_id << ", size=" << message_data.size()
                       << " bytes";

      // 基础消息验证
      if (message_data.empty()) {
        LOG_WARNING_STREAM << "[RX] 收到空消息";
        return;
      }

      // 验证消息格式
      if (!MessageFactory::ValidateMessage(message_data)) {
        LOG_WARNING_STREAM << "[RX] 消息格式验证失败";
        return;
      }

      // 创建消息对象并处理
      auto message = MessageFactory::CreateFromBytes(message_data);
      if (!message) {
        LOG_WARNING_STREAM << "[RX] 创建消息对象失败";
        return;
      }

      // 使用消息路由器处理消息
      if (service_->message_router_) {
        // 临时设置当前处理的端点ID，供回调函数使用
        service_->current_processing_endpoint_id_ = endpoint_id;

        // 调用消息的 ProcessMessage 函数，触发 message_router_ 的回调
        bool processed = message->ProcessMessage(service_->transport_, service_->message_router_);

        // 清除临时端点ID
        service_->current_processing_endpoint_id_.clear();

        if (processed) {
          LOG_DEBUG_STREAM << "[RX] 消息已通过 MessageRouter 成功处理";
          return;
        }
      }

      // 如果没有路由器，直接转发给用户事件处理器
      if (service_->event_handler_) {
        service_->event_handler_->OnMessageReceived(endpoint_id, message_data);
      }

      service_->statistics_.messages_received++;
    } catch (const std::exception &e) {
      LOG_ERROR_STREAM << "[RX] 处理传入消息失败: " << e.what();
      service_->statistics_.errors++;
    }
  }

  void OnConnectionChanged(const std::string &endpoint_id, bool connected,
                           const ConnectionInfo &connection_info) override {
    if (!service_) return;

    LOG_INFO_STREAM << "[CONN] EndpointService::OnConnectionChanged 被调用 -> service_id=" << endpoint_id
                    << ", connected=" << connected;

    // 直接更新连接状态
    {
      std::lock_guard<std::mutex> lock(service_->connections_mutex_);
      service_->endpoint_connections_[endpoint_id] = connected;
    }

    // 直接转发给用户事件处理器
    if (service_->event_handler_) {
      LOG_INFO_STREAM << "[CONN] 转发连接事件给事件处理器 -> service_id=" << endpoint_id;
      service_->event_handler_->OnConnectionChanged(endpoint_id, connected, connection_info);
    } else {
      LOG_WARNING_STREAM << "[CONN] 事件处理器为空，无法转发连接事件 -> service_id=" << endpoint_id;
    }
  }

  void OnError(const std::string &endpoint_id, uint16_t error_code, const std::string &error_message) override {
    if (!service_) return;

    // 直接更新错误统计
    service_->statistics_.errors++;

    // 直接转发给用户事件处理器
    if (service_->event_handler_) {
      service_->event_handler_->OnError(endpoint_id, error_code, error_message);
    }
  }

 private:
  EndpointService *service_;
};

// 注册心跳回调函数（统一实现）
void EndpointService::RegisterHeartbeatCallbacks() {
  if (!GetMessageRouter()) return;

  // 注册心跳请求处理回调
  GetMessageRouter()->RegisterCallback(MessageType::Request, MessageIds::HEARTBEAT_REQUEST, SubMessageIds::IDLE,
                                       [this](std::shared_ptr<ITransport> transport, uint16_t message_id,
                                              uint8_t sub_message_id, const std::vector<uint8_t> &payload) {
                                         if (!IsHeartbeatEnabled()) return;

                                         // 调用子类实现的纯虚函数
                                         OnHeartbeatRequest(transport, current_processing_endpoint_id_, message_id,
                                                            sub_message_id, payload);
                                       });

  // 注册心跳响应处理回调
  GetMessageRouter()->RegisterCallback(MessageType::Response, MessageIds::HEARTBEAT_RESPONSE, SubMessageIds::IDLE,
                                       [this](std::shared_ptr<ITransport> transport, uint16_t message_id,
                                              uint8_t sub_message_id, const std::vector<uint8_t> &payload) {
                                         if (!IsHeartbeatEnabled()) return;

                                         // 调用子类实现的纯虚函数
                                         OnHeartbeatResponse(transport, current_processing_endpoint_id_, message_id,
                                                             sub_message_id, payload);
                                       });

  LOG_DEBUG_STREAM << "[HB] 心跳回调函数已注册到 MessageRouter";
}

// 注销心跳回调函数（统一实现）
void EndpointService::UnregisterHeartbeatCallbacks() {
  if (!GetMessageRouter()) return;

  // 注销心跳回调函数
  GetMessageRouter()->UnregisterCallback(MessageType::Request, MessageIds::HEARTBEAT_REQUEST, SubMessageIds::IDLE);
  GetMessageRouter()->UnregisterCallback(MessageType::Response, MessageIds::HEARTBEAT_RESPONSE, SubMessageIds::IDLE);

  LOG_DEBUG_STREAM << "[HB] 心跳回调函数已从 MessageRouter 注销";
}
