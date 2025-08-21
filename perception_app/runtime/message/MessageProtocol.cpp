#include "IMessageProtocol.hpp"
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
        frame_.magic_id = ProtocolFrame::MAGIC_ID;
        frame_.crc16 = 0;
        frame_.message_type = 0;
        frame_.message_id = 0;
        frame_.sub_message_id = 0;
        frame_.sequence = 0;
        frame_.length = 0;
    }

    Message(MessageType message_type, uint16_t message_id, uint8_t sub_message_id)
        : status_(MessageStatus::Pending), timestamp_(std::chrono::steady_clock::now()) {
        frame_.magic_id = ProtocolFrame::MAGIC_ID;
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
    
    const ProtocolFrame& GetFrame() const override { return frame_; }
    void SetFrame(const ProtocolFrame& frame) override { frame_ = frame; }

    std::vector<uint8_t> Serialize() const override {
        return MessageFactory::BuildFrame(frame_);
    }

    bool Deserialize(const std::vector<uint8_t>& data) override {
        if (!MessageFactory::ValidateMessage(data)) {
            return false;
        }
        frame_ = MessageFactory::ParseFrame(data);
        return true;
    }

    bool Validate() const override {
        return MessageFactory::ValidateMessage(Serialize());
    }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "Message{type=" << static_cast<int>(frame_.message_type)
            << ", id=" << frame_.message_id 
            << ", sub_id=" << static_cast<int>(frame_.sub_message_id)
            << ", seq=" << frame_.sequence
            << ", length=" << frame_.length
            << ", status=" << static_cast<int>(status_) << "}";
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

// MessageFactory静态成员初始化
std::unordered_map<uint16_t, MessageFactory::Creator> MessageFactory::creators_;
std::unordered_map<uint16_t, std::string> MessageFactory::message_type_names_;
std::unordered_map<uint8_t, std::string> MessageFactory::sub_message_type_names_;
std::unordered_map<uint16_t, std::string> MessageFactory::error_code_descriptions_;
std::mutex MessageFactory::factory_mutex_;

// 初始化消息类型名称映射
void MessageFactory::InitializeMessageTypeNames() {
    std::lock_guard<std::mutex> lock(factory_mutex_);
    
    // 系统消息
    message_type_names_[MessageIds::HEARTBEAT] = "Heartbeat";
    message_type_names_[MessageIds::SERVICE_DISCOVERY] = "ServiceDiscovery";
    message_type_names_[MessageIds::CONNECTION_REQUEST] = "ConnectionRequest";
    message_type_names_[MessageIds::CONNECTION_RESPONSE] = "ConnectionResponse";
    
    // 充电枪操作消息
    message_type_names_[MessageIds::START_CHARGING] = "StartCharging";
    message_type_names_[MessageIds::STOP_CHARGING] = "StopCharging";
    message_type_names_[MessageIds::EMERGENCY_STOP] = "EmergencyStop";
    
    // 设备控制消息
    message_type_names_[MessageIds::DEVICE_CONTROL] = "DeviceControl";
    message_type_names_[MessageIds::DEVICE_STATUS] = "DeviceStatus";
    message_type_names_[MessageIds::DEVICE_CONFIG] = "DeviceConfig";
}

// 初始化子消息类型名称映射
void MessageFactory::InitializeSubMessageTypeNames() {
    std::lock_guard<std::mutex> lock(factory_mutex_);
    
    // 系统阶段
    sub_message_type_names_[SubMessageIds::IDLE] = "Idle";
    sub_message_type_names_[SubMessageIds::INITIALIZING] = "Initializing";
    sub_message_type_names_[SubMessageIds::CONNECTING] = "Connecting";
    sub_message_type_names_[SubMessageIds::READY] = "Ready";
    sub_message_type_names_[SubMessageIds::ERROR] = "Error";
    
    // 充电枪操作阶段
    sub_message_type_names_[SubMessageIds::DEVICE_SELF_CHECK] = "DeviceSelfCheck";
    sub_message_type_names_[SubMessageIds::COVER_OPERATION] = "CoverOperation";
    sub_message_type_names_[SubMessageIds::TARGET_DETECTION] = "TargetDetection";
    sub_message_type_names_[SubMessageIds::PATH_PLANNING] = "PathPlanning";
    sub_message_type_names_[SubMessageIds::INSERTION] = "Insertion";
    sub_message_type_names_[SubMessageIds::REMOVAL] = "Removal";
    sub_message_type_names_[SubMessageIds::CONNECTION_VERIFICATION] = "ConnectionVerification";
    sub_message_type_names_[SubMessageIds::COMPLETED] = "Completed";
}

// 初始化错误码描述映射
void MessageFactory::InitializeErrorCodeDescriptions() {
    std::lock_guard<std::mutex> lock(factory_mutex_);
    
    // 通用错误码
    error_code_descriptions_[ErrorCodes::SUCCESS] = "成功";
    error_code_descriptions_[ErrorCodes::GENERAL_ERROR] = "一般错误";
    error_code_descriptions_[ErrorCodes::INVALID_PARAMETER] = "无效参数";
    error_code_descriptions_[ErrorCodes::TIMEOUT] = "超时";
    error_code_descriptions_[ErrorCodes::NOT_FOUND] = "未找到";
    error_code_descriptions_[ErrorCodes::ALREADY_EXISTS] = "已存在";
    error_code_descriptions_[ErrorCodes::PERMISSION_DENIED] = "权限拒绝";
    error_code_descriptions_[ErrorCodes::RESOURCE_UNAVAILABLE] = "资源不可用";
    
    // 充电枪操作错误码
    error_code_descriptions_[ErrorCodes::DEVICE_DISCONNECTED] = "设备未连接";
    error_code_descriptions_[ErrorCodes::SELF_CHECK_FAILED] = "设备自检失败";
    error_code_descriptions_[ErrorCodes::TARGET_RECOGNITION_FAILED] = "目标识别失败";
    error_code_descriptions_[ErrorCodes::COVER_OPERATION_FAILED] = "保护盖操作失败";
    error_code_descriptions_[ErrorCodes::PATH_PLANNING_FAILED] = "路径规划失败";
    error_code_descriptions_[ErrorCodes::INSERTION_FAILED] = "插入定位失败";
    error_code_descriptions_[ErrorCodes::REMOVAL_FAILED] = "拔出操作失败";
    error_code_descriptions_[ErrorCodes::CONNECTION_FAILED] = "充电连接失败";
}

void MessageFactory::RegisterCreator(uint16_t message_id, Creator creator) {
    std::lock_guard<std::mutex> lock(factory_mutex_);
    creators_[message_id] = std::move(creator);
}

IMessage::Ptr MessageFactory::CreateMessage(MessageType message_type, uint16_t message_id, uint8_t sub_message_id) {
    std::lock_guard<std::mutex> lock(factory_mutex_);
    
    auto it = creators_.find(message_id);
    if (it != creators_.end()) {
        return it->second();
    }
    
    // 默认创建基础消息
    return std::make_shared<Message>(message_type, message_id, sub_message_id);
}

IMessage::Ptr MessageFactory::CreateFromBytes(const std::vector<uint8_t>& data) {
    if (!ValidateMessage(data)) {
        return nullptr;
    }
    
    auto frame = ParseFrame(data);
    auto message = CreateMessage(static_cast<MessageType>(frame.message_type), 
                               frame.message_id, frame.sub_message_id);
    message->SetFrame(frame);
    return message;
}

bool MessageFactory::ValidateMessage(const std::vector<uint8_t>& data) {
    if (data.size() < ProtocolFrame::MIN_FRAME_SIZE) {
        return false;
    }
    
    // 检查魔数 (2字节)
    uint16_t magic_id = static_cast<uint16_t>(data[0]) | 
                       (static_cast<uint16_t>(data[1]) << 8);
    
    if (magic_id != ProtocolFrame::MAGIC_ID) {
        return false;
    }
    
    // 检查CRC (位置在字节2-3)
    uint16_t stored_crc = static_cast<uint16_t>(data[2]) | (static_cast<uint16_t>(data[3]) << 8);
    uint16_t calculated_crc = CalculateCRC16(data.data() + 4, data.size() - 4);
    
    return stored_crc == calculated_crc;
}

std::vector<uint8_t> MessageFactory::CreateHeartbeatMessage() {
    // 创建符合协议帧格式的心跳消息
    ProtocolFrame frame;
    frame.magic_id = ProtocolFrame::MAGIC_ID;
    frame.message_type = 0xFF; // 心跳消息类型
    frame.message_id = 0xFFFF; // 心跳消息ID
    frame.sub_message_id = 0xFF; // 心跳子消息ID
    frame.sequence = 0;
    frame.length = 17; // "HEARTBEAT" + 8字节时间戳
    
    // 心跳负载：统一心跳关键字 "HEARTBEAT" + 8字节时间戳（小端）
    std::vector<uint8_t> payload = {'H','E','A','R','T','B','E','A','T'};
    auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    for (int i = 0; i < 8; ++i) payload.push_back((ts >> (i * 8)) & 0xFF);
    frame.payload = payload;
    
    return BuildFrame(frame);
}

uint16_t MessageFactory::CalculateCRC16(const std::vector<uint8_t>& data) {
    return CalculateCRC16(data.data(), data.size());
}

uint16_t MessageFactory::CalculateCRC16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return crc;
}

bool MessageFactory::VerifyCRC16(const std::vector<uint8_t>& data, uint16_t crc16) {
    return CalculateCRC16(data) == crc16;
}

ProtocolFrame MessageFactory::ParseFrame(const std::vector<uint8_t>& data) {
    ProtocolFrame frame;
    
    if (data.size() >= ProtocolFrame::HEADER_SIZE) {
        frame.magic_id = static_cast<uint32_t>(data[0]) | 
                        (static_cast<uint32_t>(data[1]) << 8) |
                        (static_cast<uint32_t>(data[2]) << 16) |
                        (static_cast<uint32_t>(data[3]) << 24);
        frame.crc16 = static_cast<uint16_t>(data[2]) | (static_cast<uint16_t>(data[3]) << 8);
        frame.message_type = data[4];
        frame.message_id = static_cast<uint16_t>(data[5]) | (static_cast<uint16_t>(data[6]) << 8);
        frame.sub_message_id = data[7];
        frame.sequence = static_cast<uint16_t>(data[8]) | (static_cast<uint16_t>(data[9]) << 8);
        frame.length = static_cast<uint16_t>(data[10]) | (static_cast<uint16_t>(data[11]) << 8);
        
        if (data.size() >= ProtocolFrame::HEADER_SIZE + frame.length) {
            frame.payload.assign(data.begin() + ProtocolFrame::HEADER_SIZE, 
                               data.begin() + ProtocolFrame::HEADER_SIZE + frame.length);
        }
    }
    
    return frame;
}

std::vector<uint8_t> MessageFactory::BuildFrame(const ProtocolFrame& frame) {
    std::vector<uint8_t> data;
    
    // 添加魔数 (2字节)
    data.push_back(static_cast<uint8_t>(frame.magic_id & 0xFF));
    data.push_back(static_cast<uint8_t>((frame.magic_id >> 8) & 0xFF));
    
    // 预留CRC位置 (2字节)
    data.push_back(0);
    data.push_back(0);
    
    // 添加其他字段
    data.push_back(frame.message_type);
    data.push_back(static_cast<uint8_t>(frame.message_id & 0xFF));
    data.push_back(static_cast<uint8_t>((frame.message_id >> 8) & 0xFF));
    data.push_back(frame.sub_message_id);
    data.push_back(static_cast<uint8_t>(frame.sequence & 0xFF));
    data.push_back(static_cast<uint8_t>((frame.sequence >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(frame.length & 0xFF));
    data.push_back(static_cast<uint8_t>((frame.length >> 8) & 0xFF));
    
    // 添加负载
    data.insert(data.end(), frame.payload.begin(), frame.payload.end());
    
    // 计算并设置CRC (位置在字节2-3)
    uint16_t crc = CalculateCRC16(data.data() + 4, data.size() - 4);
    data[2] = static_cast<uint8_t>(crc & 0xFF);
    data[3] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    
    return data;
}

std::string MessageFactory::GetMessageTypeName(uint16_t message_id) {
    std::lock_guard<std::mutex> lock(factory_mutex_);
    auto it = message_type_names_.find(message_id);
    return it != message_type_names_.end() ? it->second : "Unknown";
}

std::string MessageFactory::GetSubMessageTypeName(uint8_t sub_message_id) {
    std::lock_guard<std::mutex> lock(factory_mutex_);
    auto it = sub_message_type_names_.find(sub_message_id);
    return it != sub_message_type_names_.end() ? it->second : "Unknown";
}

std::string MessageFactory::GetErrorCodeDescription(uint16_t error_code) {
    std::lock_guard<std::mutex> lock(factory_mutex_);
    auto it = error_code_descriptions_.find(error_code);
    return it != error_code_descriptions_.end() ? it->second : "未知错误";
}

// MessageRouter实现
class MessageRouter : public IMessageRouter {
public:
    void RegisterHandler(uint16_t message_id, IMessageHandler::Ptr handler) override {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        handlers_[message_id] = std::move(handler);
    }

    void UnregisterHandler(uint16_t message_id) override {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        handlers_.erase(message_id);
    }

    IMessage::Ptr RouteMessage(const IMessage::Ptr& message) override {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        
        auto it = handlers_.find(message->GetMessageId());
        if (it != handlers_.end()) {
            return it->second->HandleMessage(message);
        }
        
        return nullptr;
    }

    bool HasHandler(uint16_t message_id) const override {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        return handlers_.find(message_id) != handlers_.end();
    }

    size_t GetHandlerCount() const override {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        return handlers_.size();
    }

private:
    std::unordered_map<uint16_t, IMessageHandler::Ptr> handlers_;
    mutable std::mutex handlers_mutex_;
};

// 创建MessageRouter实例的工厂函数
IMessageRouter::Ptr CreateMessageRouter() {
    return std::make_shared<MessageRouter>();
}

} // namespace perception
