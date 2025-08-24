#include "IMessageProtocol.hpp"
#include "ProtocolDefinitions.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <mutex>

namespace perception {

// Message类实现
class Message : public IMessage {
 public:
  Message() : status_(MessageStatus::Pending), timestamp_(std::chrono::steady_clock::now()) {
    frame_.magic_id = ProtocolConstants::MAGIC_ID;
    frame_.crc16 = 0;
    frame_.message_type = 0;
    frame_.message_id = 0;
    frame_.sub_message_id = 0;
    frame_.sequence = 0;
    frame_.length = 0;
  }

  Message(MessageType message_type, uint16_t message_id, uint8_t sub_message_id)
      : status_(MessageStatus::Pending), timestamp_(std::chrono::steady_clock::now()) {
    frame_.magic_id = ProtocolConstants::MAGIC_ID;
    frame_.crc16 = 0;
    frame_.message_type = static_cast<uint8_t>(message_type);
    frame_.message_id = message_id;
    frame_.sub_message_id = sub_message_id;
    frame_.sequence = 0;
    frame_.length = 0;
  }

  // IMessage接口实现
  MessageType GetType() const override { return static_cast<MessageType>(frame_.message_type); }
  uint16_t GetMessageId() const override { return frame_.message_id; }
  uint8_t GetSubMessageId() const override { return frame_.sub_message_id; }
  uint16_t GetSequence() const override { return frame_.sequence; }
  uint64_t GetTimestamp() const override {
    return std::chrono::duration_cast<std::chrono::milliseconds>(timestamp_.time_since_epoch()).count();
  }
  MessageStatus GetStatus() const override { return status_; }
  void SetStatus(MessageStatus status) override { status_ = status; }

  const ProtocolFrame &GetFrame() const override { return frame_; }
  void SetFrame(const ProtocolFrame &frame) override { frame_ = frame; }

  std::vector<uint8_t> Serialize() const override { return ProtocolUtils::BuildFrame(frame_); }

  bool Deserialize(const std::vector<uint8_t> &data) override {
    if (!ProtocolUtils::ValidateMessage(data)) {
      return false;
    }
    frame_ = ProtocolUtils::ParseFrame(data);
    return true;
  }

  bool Validate() const override { return ProtocolUtils::ValidateMessage(Serialize()); }

  bool ProcessMessage(std::shared_ptr<ITransport> transport, std::shared_ptr<MessageRouter> router) override {
    if (!router) {
      return false;
    }

    // 直接通过路由器路由消息
    return router->InvokeCallback(transport, GetType(), GetMessageId(), GetSubMessageId(), frame_.payload);
  }

  std::string ToString() const override {
    std::ostringstream oss;
    oss << "Message{type=" << static_cast<int>(frame_.message_type) << ", id=" << frame_.message_id
        << ", sub_id=" << static_cast<int>(frame_.sub_message_id) << ", seq=" << frame_.sequence
        << ", length=" << frame_.length << ", status=" << static_cast<int>(status_) << "}";
    return oss.str();
  }

  std::string ToHexString() const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    auto data = Serialize();
    for (size_t i = 0; i < data.size(); ++i) {
      if (i > 0) oss << " ";
      oss << std::setw(2) << static_cast<int>(data[i]);
    }

    return oss.str();
  }

 private:
  ProtocolFrame frame_;
  MessageStatus status_;
  std::chrono::steady_clock::time_point timestamp_;
};

// MessageRouter实现
void MessageRouter::RegisterCallback(MessageType message_type, uint16_t message_id, uint8_t sub_message_id,
                                     MessageCallback callback) {
  std::lock_guard<std::mutex> lock(router_mutex_);
  MessageKey key(message_type, message_id, sub_message_id);
  callbacks_[key] = std::move(callback);
}

void MessageRouter::UnregisterCallback(MessageType message_type, uint16_t message_id, uint8_t sub_message_id) {
  std::lock_guard<std::mutex> lock(router_mutex_);
  MessageKey key(message_type, message_id, sub_message_id);
  callbacks_.erase(key);
}

bool MessageRouter::InvokeCallback(std::shared_ptr<ITransport> transport, MessageType message_type, uint16_t message_id,
                                   uint8_t sub_message_id, const std::vector<uint8_t> &payload) {
  std::lock_guard<std::mutex> lock(router_mutex_);
  MessageKey key(message_type, message_id, sub_message_id);
  auto it = callbacks_.find(key);
  if (it != callbacks_.end() && it->second) {
    it->second(transport, message_id, sub_message_id, payload);
    return true;
  }
  return false;
}

bool MessageRouter::HasCallback(MessageType message_type, uint16_t message_id, uint8_t sub_message_id) const {
  std::lock_guard<std::mutex> lock(router_mutex_);
  MessageKey key(message_type, message_id, sub_message_id);
  return callbacks_.find(key) != callbacks_.end();
}

size_t MessageRouter::GetCallbackCount() const {
  std::lock_guard<std::mutex> lock(router_mutex_);
  return callbacks_.size();
}

void MessageRouter::Clear() {
  std::lock_guard<std::mutex> lock(router_mutex_);
  callbacks_.clear();
}

void MessageRouter::InitializeDefaultRoutes() {
  // 直接操作 callbacks_ 映射，避免死锁
  std::lock_guard<std::mutex> lock(router_mutex_);

  // 心跳消息路由（现在由 EndpointService 注册，这里只保留占位符）
  MessageKey hb_request_key(MessageType::Request, MessageIds::HEARTBEAT_REQUEST, SubMessageIds::IDLE);
  callbacks_[hb_request_key] = [](std::shared_ptr<ITransport> transport, uint16_t message_id, uint8_t sub_message_id,
                                  const std::vector<uint8_t> &payload) {
    std::cout << "[HB] 收到心跳请求消息（占位符）" << std::endl;
    // 实际处理由 EndpointService 注册的回调函数完成
  };

  MessageKey hb_response_key(MessageType::Response, MessageIds::HEARTBEAT_RESPONSE, SubMessageIds::IDLE);
  callbacks_[hb_response_key] = [](std::shared_ptr<ITransport> transport, uint16_t message_id, uint8_t sub_message_id,
                                   const std::vector<uint8_t> &payload) {
    std::cout << "[HB] 收到心跳响应消息（占位符）" << std::endl;
    // 实际处理由 EndpointService 注册的回调函数完成
  };

  // 充电操作消息路由
  MessageKey start_charging_key(MessageType::Request, MessageIds::START_CHARGING, SubMessageIds::IDLE);
  callbacks_[start_charging_key] = [](std::shared_ptr<ITransport> transport, uint16_t message_id,
                                      uint8_t sub_message_id, const std::vector<uint8_t> &payload) {
    std::cout << "[CHARGING] 收到开始充电请求" << std::endl;
    // 这里可以添加开始充电的具体处理逻辑
  };

  MessageKey stop_charging_key(MessageType::Request, MessageIds::STOP_CHARGING, SubMessageIds::IDLE);
  callbacks_[stop_charging_key] = [](std::shared_ptr<ITransport> transport, uint16_t message_id, uint8_t sub_message_id,
                                     const std::vector<uint8_t> &payload) {
    std::cout << "[CHARGING] 收到停止充电请求" << std::endl;
    // 这里可以添加停止充电的具体处理逻辑
  };

  MessageKey emergency_stop_key(MessageType::Request, MessageIds::EMERGENCY_STOP, SubMessageIds::IDLE);
  callbacks_[emergency_stop_key] = [](std::shared_ptr<ITransport> transport, uint16_t message_id,
                                      uint8_t sub_message_id, const std::vector<uint8_t> &payload) {
    std::cout << "[EMERGENCY] 收到紧急停止请求" << std::endl;
    // 这里可以添加紧急停止的具体处理逻辑
  };

  // 设备控制消息路由
  MessageKey device_control_key(MessageType::Request, MessageIds::DEVICE_CONTROL, SubMessageIds::IDLE);
  callbacks_[device_control_key] = [](std::shared_ptr<ITransport> transport, uint16_t message_id,
                                      uint8_t sub_message_id, const std::vector<uint8_t> &payload) {
    std::cout << "[DEVICE] 收到设备控制请求" << std::endl;
    // 这里可以添加设备控制的具体处理逻辑
  };

  MessageKey device_status_key(MessageType::Notify, MessageIds::DEVICE_STATUS, SubMessageIds::IDLE);
  callbacks_[device_status_key] = [](std::shared_ptr<ITransport> transport, uint16_t message_id, uint8_t sub_message_id,
                                     const std::vector<uint8_t> &payload) {
    std::cout << "[DEVICE] 收到设备状态通知" << std::endl;
    // 这里可以添加设备状态处理的具体逻辑
  };

  // 连接管理消息路由
  MessageKey connection_request_key(MessageType::Request, MessageIds::CONNECTION_REQUEST, SubMessageIds::IDLE);
  callbacks_[connection_request_key] = [](std::shared_ptr<ITransport> transport, uint16_t message_id,
                                          uint8_t sub_message_id, const std::vector<uint8_t> &payload) {
    std::cout << "[CONNECTION] 收到连接请求" << std::endl;
    // 这里可以添加连接请求的具体处理逻辑
  };

  MessageKey connection_response_key(MessageType::Response, MessageIds::CONNECTION_RESPONSE, SubMessageIds::IDLE);
  callbacks_[connection_response_key] = [](std::shared_ptr<ITransport> transport, uint16_t message_id,
                                           uint8_t sub_message_id, const std::vector<uint8_t> &payload) {
    std::cout << "[CONNECTION] 收到连接响应" << std::endl;
    // 这里可以添加连接响应的具体处理逻辑
  };

  std::cout << "[ROUTER] 默认消息路由表初始化完成，共注册 " << callbacks_.size() << " 个回调函数" << std::endl;
}

// MessageFactory静态成员初始化
std::unordered_map<uint16_t, MessageCallback> MessageFactory::message_callbacks_;
std::mutex MessageFactory::factory_mutex_;

IMessage::Ptr MessageFactory::CreateFromBytes(const std::vector<uint8_t> &data) {
  if (!ProtocolUtils::ValidateMessage(data)) {
    return nullptr;
  }

  auto frame = ProtocolUtils::ParseFrame(data);
  auto message =
      std::make_shared<Message>(static_cast<MessageType>(frame.message_type), frame.message_id, frame.sub_message_id);
  message->SetFrame(frame);
  return message;
}

bool MessageFactory::ValidateMessage(const std::vector<uint8_t> &data) { return ProtocolUtils::ValidateMessage(data); }

std::vector<uint8_t> MessageFactory::CreateHeartbeatRequestMessage() {
  // 创建心跳请求消息（服务器发起）
  ProtocolFrame frame;
  frame.magic_id = ProtocolConstants::MAGIC_ID;
  frame.message_type = static_cast<uint8_t>(MessageType::Request);
  frame.message_id = MessageIds::HEARTBEAT_REQUEST;
  frame.sub_message_id = SubMessageIds::IDLE;
  frame.sequence = 0;
  frame.length = 17;  // "HEARTBEAT" + 8字节时间戳

  // 心跳负载：统一心跳关键字 "HEARTBEAT" + 8字节时间戳（小端）
  std::vector<uint8_t> payload = {'H', 'E', 'A', 'R', 'T', 'B', 'E', 'A', 'T'};
  auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
                .count();
  for (int i = 0; i < 8; ++i) payload.push_back((ts >> (i * 8)) & 0xFF);
  frame.payload = payload;

  return ProtocolUtils::BuildFrame(frame);
}

std::vector<uint8_t> MessageFactory::CreateHeartbeatResponseMessage() {
  // 创建心跳响应消息（客户端回复）
  ProtocolFrame frame;
  frame.magic_id = ProtocolConstants::MAGIC_ID;
  frame.message_type = static_cast<uint8_t>(MessageType::Response);
  frame.message_id = MessageIds::HEARTBEAT_RESPONSE;
  frame.sub_message_id = SubMessageIds::IDLE;
  frame.sequence = 0;
  frame.length = 17;  // "HEARTBEAT" + 8字节时间戳

  // 心跳负载：统一心跳关键字 "HEARTBEAT" + 8字节时间戳（小端）
  std::vector<uint8_t> payload = {'H', 'E', 'A', 'R', 'T', 'B', 'E', 'A', 'T'};
  auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
                .count();
  for (int i = 0; i < 8; ++i) payload.push_back((ts >> (i * 8)) & 0xFF);
  frame.payload = payload;

  return ProtocolUtils::BuildFrame(frame);
}

bool MessageFactory::IsHeartbeatRequestMessage(const std::vector<uint8_t> &message_data) {
  // 检查是否是心跳请求消息
  if (message_data.size() < ProtocolConstants::HEADER_SIZE + 17) return false;

  // 检查魔数 (2字节)
  uint16_t magic_id = static_cast<uint16_t>(message_data[ProtocolConstants::MAGIC_OFFSET]) |
                      (static_cast<uint16_t>(message_data[ProtocolConstants::MAGIC_OFFSET + 1]) << 8);
  if (magic_id != ProtocolConstants::MAGIC_ID) return false;

  // 检查消息类型和ID
  if (message_data[ProtocolConstants::MSG_TYPE_OFFSET] != static_cast<uint8_t>(MessageType::Request)) return false;

  uint16_t message_id = static_cast<uint16_t>(message_data[ProtocolConstants::MSG_ID_OFFSET]) |
                        (static_cast<uint16_t>(message_data[ProtocolConstants::MSG_ID_OFFSET + 1]) << 8);
  if (message_id != MessageIds::HEARTBEAT_REQUEST) return false;

  // 检查负载中的心跳关键字
  const char key[] = {'H', 'E', 'A', 'R', 'T', 'B', 'E', 'A', 'T'};
  for (size_t i = 0; i < 9; ++i) {
    if (message_data[ProtocolConstants::PAYLOAD_OFFSET + i] != (uint8_t)key[i]) return false;
  }

  return true;
}

bool MessageFactory::IsHeartbeatResponseMessage(const std::vector<uint8_t> &message_data) {
  // 检查是否是心跳响应消息
  if (message_data.size() < ProtocolConstants::HEADER_SIZE + 17) return false;

  // 检查魔数 (2字节)
  uint16_t magic_id = static_cast<uint16_t>(message_data[ProtocolConstants::MAGIC_OFFSET]) |
                      (static_cast<uint16_t>(message_data[ProtocolConstants::MAGIC_OFFSET + 1]) << 8);
  if (magic_id != ProtocolConstants::MAGIC_ID) return false;

  // 检查消息类型和ID
  if (message_data[ProtocolConstants::MSG_TYPE_OFFSET] != static_cast<uint8_t>(MessageType::Response)) return false;

  uint16_t message_id = static_cast<uint16_t>(message_data[ProtocolConstants::MSG_ID_OFFSET]) |
                        (static_cast<uint16_t>(message_data[ProtocolConstants::MSG_ID_OFFSET + 1]) << 8);
  if (message_id != MessageIds::HEARTBEAT_RESPONSE) return false;

  // 检查负载中的心跳关键字
  const char key[] = {'H', 'E', 'A', 'R', 'T', 'B', 'E', 'A', 'T'};
  for (size_t i = 0; i < 9; ++i) {
    if (message_data[ProtocolConstants::PAYLOAD_OFFSET + i] != (uint8_t)key[i]) return false;
  }

  return true;
}

bool MessageFactory::IsHeartbeatMessage(const std::vector<uint8_t> &message_data) {
  // 检查是否是心跳消息（请求或响应）
  return IsHeartbeatRequestMessage(message_data) || IsHeartbeatResponseMessage(message_data);
}

// 消息回调相关方法实现
void MessageFactory::RegisterMessageCallback(uint16_t message_id, MessageCallback callback) {
  std::lock_guard<std::mutex> lock(factory_mutex_);
  message_callbacks_[message_id] = std::move(callback);
}

void MessageFactory::UnregisterMessageCallback(uint16_t message_id) {
  std::lock_guard<std::mutex> lock(factory_mutex_);
  message_callbacks_.erase(message_id);
}

bool MessageFactory::ProcessMessageData(std::shared_ptr<ITransport> transport, std::shared_ptr<MessageRouter> router,
                                        const std::vector<uint8_t> &message_data) {
  if (!transport || !router) {
    return false;
  }

  // 验证消息格式
  if (!ValidateMessage(message_data)) {
    std::cout << "[FACTORY] 消息格式验证失败" << std::endl;
    return false;
  }

  // 创建消息对象
  auto message = CreateFromBytes(message_data);
  if (!message) {
    std::cout << "[FACTORY] 创建消息对象失败" << std::endl;
    return false;
  }

  // 处理消息
  return message->ProcessMessage(transport, router);
}

// 便捷的消息创建函数实现
std::vector<uint8_t> MessageFactory::CreateRequestMessage(uint16_t message_id, uint8_t sub_message_id,
                                                          const std::vector<uint8_t> &payload) {
  ProtocolFrame frame;
  frame.magic_id = ProtocolConstants::MAGIC_ID;
  frame.message_type = static_cast<uint8_t>(MessageType::Request);
  frame.message_id = message_id;
  frame.sub_message_id = sub_message_id;
  frame.sequence = 0;
  frame.payload = payload;
  frame.length = payload.size();

  return ProtocolUtils::BuildFrame(frame);
}

std::vector<uint8_t> MessageFactory::CreateResponseMessage(uint16_t message_id, uint8_t sub_message_id,
                                                           const std::vector<uint8_t> &payload) {
  ProtocolFrame frame;
  frame.magic_id = ProtocolConstants::MAGIC_ID;
  frame.message_type = static_cast<uint8_t>(MessageType::Response);
  frame.message_id = message_id;
  frame.sub_message_id = sub_message_id;
  frame.sequence = 0;
  frame.payload = payload;
  frame.length = payload.size();

  return ProtocolUtils::BuildFrame(frame);
}

std::vector<uint8_t> MessageFactory::CreateNotifyMessage(uint16_t message_id, uint8_t sub_message_id,
                                                         const std::vector<uint8_t> &payload) {
  ProtocolFrame frame;
  frame.magic_id = ProtocolConstants::MAGIC_ID;
  frame.message_type = static_cast<uint8_t>(MessageType::Notify);
  frame.message_id = message_id;
  frame.sub_message_id = sub_message_id;
  frame.sequence = 0;
  frame.payload = payload;
  frame.length = payload.size();

  return ProtocolUtils::BuildFrame(frame);
}

// 字符串负载版本的便捷函数
std::vector<uint8_t> MessageFactory::CreateRequestMessage(uint16_t message_id, uint8_t sub_message_id,
                                                          const std::string &payload_str) {
  std::vector<uint8_t> payload(payload_str.begin(), payload_str.end());
  return CreateRequestMessage(message_id, sub_message_id, payload);
}

std::vector<uint8_t> MessageFactory::CreateResponseMessage(uint16_t message_id, uint8_t sub_message_id,
                                                           const std::string &payload_str) {
  std::vector<uint8_t> payload(payload_str.begin(), payload_str.end());
  return CreateResponseMessage(message_id, sub_message_id, payload);
}

std::vector<uint8_t> MessageFactory::CreateNotifyMessage(uint16_t message_id, uint8_t sub_message_id,
                                                         const std::string &payload_str) {
  std::vector<uint8_t> payload(payload_str.begin(), payload_str.end());
  return CreateNotifyMessage(message_id, sub_message_id, payload);
}

std::string MessageFactory::GetMessageTypeName(uint16_t message_id) {
  // 使用宏函数自动生成字符串映射
  switch (message_id) {
    case MessageIds::HEARTBEAT_REQUEST:
      return ENUM_TO_STRING(HEARTBEAT_REQUEST);
    case MessageIds::HEARTBEAT_RESPONSE:
      return ENUM_TO_STRING(HEARTBEAT_RESPONSE);
    case MessageIds::SERVICE_DISCOVERY:
      return ENUM_TO_STRING(SERVICE_DISCOVERY);
    case MessageIds::CONNECTION_REQUEST:
      return ENUM_TO_STRING(CONNECTION_REQUEST);
    case MessageIds::CONNECTION_RESPONSE:
      return ENUM_TO_STRING(CONNECTION_RESPONSE);
    case MessageIds::START_CHARGING:
      return ENUM_TO_STRING(START_CHARGING);
    case MessageIds::STOP_CHARGING:
      return ENUM_TO_STRING(STOP_CHARGING);
    case MessageIds::EMERGENCY_STOP:
      return ENUM_TO_STRING(EMERGENCY_STOP);
    case MessageIds::DEVICE_CONTROL:
      return ENUM_TO_STRING(DEVICE_CONTROL);
    case MessageIds::DEVICE_STATUS:
      return ENUM_TO_STRING(DEVICE_STATUS);
    case MessageIds::DEVICE_CONFIG:
      return ENUM_TO_STRING(DEVICE_CONFIG);
    default:
      return "Unknown";
  }
}

std::string MessageFactory::GetSubMessageTypeName(uint8_t sub_message_id) {
  // 使用宏函数自动生成字符串映射
  switch (sub_message_id) {
    case SubMessageIds::IDLE:
      return ENUM_TO_STRING(IDLE);
    case SubMessageIds::INITIALIZING:
      return ENUM_TO_STRING(INITIALIZING);
    case SubMessageIds::CONNECTING:
      return ENUM_TO_STRING(CONNECTING);
    case SubMessageIds::READY:
      return ENUM_TO_STRING(READY);
    case SubMessageIds::ERROR:
      return ENUM_TO_STRING(ERROR);
    case SubMessageIds::DEVICE_SELF_CHECK:
      return ENUM_TO_STRING(DEVICE_SELF_CHECK);
    case SubMessageIds::COVER_OPERATION:
      return ENUM_TO_STRING(COVER_OPERATION);
    case SubMessageIds::TARGET_DETECTION:
      return ENUM_TO_STRING(TARGET_DETECTION);
    case SubMessageIds::PATH_PLANNING:
      return ENUM_TO_STRING(PATH_PLANNING);
    case SubMessageIds::INSERTION:
      return ENUM_TO_STRING(INSERTION);
    case SubMessageIds::REMOVAL:
      return ENUM_TO_STRING(REMOVAL);
    case SubMessageIds::CONNECTION_VERIFICATION:
      return ENUM_TO_STRING(CONNECTION_VERIFICATION);
    case SubMessageIds::COMPLETED:
      return ENUM_TO_STRING(COMPLETED);
    default:
      return "Unknown";
  }
}

std::string MessageFactory::GetErrorCodeDescription(uint16_t error_code) {
  // 使用宏函数自动生成字符串映射
  switch (error_code) {
    case ErrorCodes::SUCCESS:
      return "成功";
    case ErrorCodes::GENERAL_ERROR:
      return "一般错误";
    case ErrorCodes::INVALID_PARAMETER:
      return "无效参数";
    case ErrorCodes::TIMEOUT:
      return "超时";
    case ErrorCodes::NOT_FOUND:
      return "未找到";
    case ErrorCodes::ALREADY_EXISTS:
      return "已存在";
    case ErrorCodes::PERMISSION_DENIED:
      return "权限拒绝";
    case ErrorCodes::RESOURCE_UNAVAILABLE:
      return "资源不可用";
    case ErrorCodes::DEVICE_DISCONNECTED:
      return "设备未连接";
    case ErrorCodes::SELF_CHECK_FAILED:
      return "设备自检失败";
    case ErrorCodes::TARGET_RECOGNITION_FAILED:
      return "目标识别失败";
    case ErrorCodes::COVER_OPERATION_FAILED:
      return "保护盖操作失败";
    case ErrorCodes::PATH_PLANNING_FAILED:
      return "路径规划失败";
    case ErrorCodes::INSERTION_FAILED:
      return "插入定位失败";
    case ErrorCodes::REMOVAL_FAILED:
      return "拔出操作失败";
    case ErrorCodes::CONNECTION_FAILED:
      return "充电连接失败";
    default:
      return "未知错误";
  }
}

}  // namespace perception
