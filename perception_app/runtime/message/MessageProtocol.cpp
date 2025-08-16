#include "MessageProtocol.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace perception {

// Message类实现 - 只实现头文件中声明但未实现的方法

Message::Message() : status_(MessageStatus::Pending), timestamp_(std::chrono::steady_clock::now()) {
    frame_.magic_id = ProtocolFrame::MAGIC_ID;
    frame_.crc16 = 0;
    frame_.message_type = 0;
    frame_.message_id = 0;
    frame_.sub_message_id = 0;
    frame_.sequence = 0;
    frame_.length = 0;
}

Message::Message(uint8_t message_type, uint16_t message_id, uint8_t sub_message_id)
    : status_(MessageStatus::Pending), timestamp_(std::chrono::steady_clock::now()) {
    frame_.magic_id = ProtocolFrame::MAGIC_ID;
    frame_.crc16 = 0;
    frame_.message_type = message_type;
    frame_.message_id = message_id;
    frame_.sub_message_id = sub_message_id;
    frame_.sequence = 0;
    frame_.length = 0;
}

void Message::SetPayload(const std::vector<uint8_t>& payload) {
    frame_.payload = payload;
    frame_.length = static_cast<uint16_t>(payload.size());
}

std::string Message::GetMessageTypeString() const {
    return MessageFactory::GetMessageTypeName(frame_.message_id);
}

std::string Message::GetSubMessageTypeString() const {
    return MessageFactory::GetSubMessageTypeName(frame_.sub_message_id);
}

std::string Message::ToString() const {
    std::ostringstream oss;
    oss << "Message{type=" << static_cast<int>(frame_.message_type)
        << ", id=" << frame_.message_id 
        << ", sub_id=" << static_cast<int>(frame_.sub_message_id)
        << ", seq=" << frame_.sequence
        << ", length=" << frame_.length
        << ", status=" << static_cast<int>(status_) << "}";
    return oss.str();
}

std::string Message::ToHexString() const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    
    // 添加帧头
    oss << std::setw(4) << frame_.magic_id << " ";
    oss << std::setw(4) << frame_.crc16 << " ";
    oss << std::setw(2) << static_cast<int>(frame_.message_type) << " ";
    oss << std::setw(4) << frame_.message_id << " ";
    oss << std::setw(2) << static_cast<int>(frame_.sub_message_id) << " ";
    oss << std::setw(4) << frame_.sequence << " ";
    oss << std::setw(4) << frame_.length << " ";
    
    // 添加负载数据
    for (uint8_t byte : frame_.payload) {
        oss << std::setw(2) << static_cast<int>(byte) << " ";
    }
    
    return oss.str();
}

// 序列化实现
std::vector<uint8_t> Message::Serialize() const {
    return BuildFrame(frame_);
}

// 反序列化实现
bool Message::Deserialize(const std::vector<uint8_t>& data) {
    if (!ValidateFrame(data)) {
        return false;
    }
    
    frame_ = ParseFrame(data);
    return true;
}

// 静态方法实现
bool Message::ValidateFrame(const std::vector<uint8_t>& data) {
    if (data.size() < 16) { // 最小帧长度
        return false;
    }
    
    // 检查魔数
    uint32_t magic_id = static_cast<uint32_t>(data[0]) | 
                       (static_cast<uint32_t>(data[1]) << 8) |
                       (static_cast<uint32_t>(data[2]) << 16) |
                       (static_cast<uint32_t>(data[3]) << 24);
    
    if (magic_id != ProtocolFrame::MAGIC_ID) {
        return false;
    }
    
    // 检查CRC
    uint16_t stored_crc = static_cast<uint16_t>(data[4]) | (static_cast<uint16_t>(data[5]) << 8);
    uint16_t calculated_crc = CalculateCRC16(data.data() + 6, data.size() - 6);
    
    return stored_crc == calculated_crc;
}

uint16_t Message::CalculateCRC16(const std::vector<uint8_t>& data) {
    return CalculateCRC16(data.data(), data.size());
}

uint16_t Message::CalculateCRC16(const uint8_t* data, size_t length) {
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

bool Message::VerifyCRC16(const std::vector<uint8_t>& data, uint16_t crc16) {
    return CalculateCRC16(data) == crc16;
}

ProtocolFrame Message::ParseFrame(const std::vector<uint8_t>& data) {
    ProtocolFrame frame;
    
    if (data.size() >= 16) {
        frame.magic_id = static_cast<uint32_t>(data[0]) | 
                        (static_cast<uint32_t>(data[1]) << 8) |
                        (static_cast<uint32_t>(data[2]) << 16) |
                        (static_cast<uint32_t>(data[3]) << 24);
        frame.crc16 = static_cast<uint16_t>(data[4]) | (static_cast<uint16_t>(data[5]) << 8);
        frame.message_type = data[6];
        frame.message_id = static_cast<uint16_t>(data[7]) | (static_cast<uint16_t>(data[8]) << 8);
        frame.sub_message_id = data[9];
        frame.sequence = static_cast<uint16_t>(data[10]) | (static_cast<uint16_t>(data[11]) << 8);
        frame.length = static_cast<uint16_t>(data[12]) | (static_cast<uint16_t>(data[13]) << 8);
        
        if (data.size() >= 16 + frame.length) {
            frame.payload.assign(data.begin() + 16, data.begin() + 16 + frame.length);
        }
    }
    
    return frame;
}

std::vector<uint8_t> Message::BuildFrame(const ProtocolFrame& frame) {
    std::vector<uint8_t> data;
    
    // 添加帧头（除了CRC）
    data.push_back(static_cast<uint8_t>(frame.magic_id & 0xFF));
    data.push_back(static_cast<uint8_t>((frame.magic_id >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((frame.magic_id >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((frame.magic_id >> 24) & 0xFF));
    
    // 预留CRC位置
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
    
    // 计算并设置CRC
    uint16_t crc = CalculateCRC16(data.data() + 6, data.size() - 6);
    data[4] = static_cast<uint8_t>(crc & 0xFF);
    data[5] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    
    return data;
}

// RequestMessage实现
RequestMessage::RequestMessage(uint16_t message_id, uint8_t sub_message_id)
    : Message(1, message_id, sub_message_id) {
}

void RequestMessage::SetParameters(const std::vector<uint8_t>& params) {
    SetRequestData(params);
}

std::vector<uint8_t> RequestMessage::GetParameters() const {
    return GetRequestData();
}

void RequestMessage::SetRequestData(const std::vector<uint8_t>& data) {
    SetPayload(data);
}

std::vector<uint8_t> RequestMessage::GetRequestData() const {
    return GetPayload();
}

std::string RequestMessage::ToString() const {
    std::ostringstream oss;
    oss << "RequestMessage{id=" << GetMessageId() 
        << ", sub_id=" << static_cast<int>(GetSubMessageId())
        << ", data_size=" << GetPayload().size() << "}";
    return oss.str();
}

// ResponseMessage实现
ResponseMessage::ResponseMessage(uint16_t message_id, uint8_t sub_message_id)
    : Message(2, message_id, sub_message_id), error_code_(0) {
}

void ResponseMessage::SetResult(const std::vector<uint8_t>& result) {
    SetPayload(result);
}

std::vector<uint8_t> ResponseMessage::GetResult() const {
    return GetPayload();
}

void ResponseMessage::SetErrorCode(uint16_t error_code) {
    error_code_ = error_code;
}

uint16_t ResponseMessage::GetErrorCode() const {
    return error_code_;
}

std::string ResponseMessage::ToString() const {
    std::ostringstream oss;
    oss << "ResponseMessage{id=" << GetMessageId() 
        << ", sub_id=" << static_cast<int>(GetSubMessageId())
        << ", error_code=" << error_code_
        << ", data_size=" << GetPayload().size() << "}";
    return oss.str();
}

// NotifyMessage实现
NotifyMessage::NotifyMessage(uint16_t message_id, uint8_t sub_message_id)
    : Message(3, message_id, sub_message_id) {
}

void NotifyMessage::SetNotificationData(const std::vector<uint8_t>& data) {
    SetPayload(data);
}

std::vector<uint8_t> NotifyMessage::GetNotificationData() const {
    return GetPayload();
}

std::string NotifyMessage::ToString() const {
    std::ostringstream oss;
    oss << "NotifyMessage{id=" << GetMessageId() 
        << ", sub_id=" << static_cast<int>(GetSubMessageId())
        << ", data_size=" << GetPayload().size() << "}";
    return oss.str();
}

// MessageFactory实现
std::unordered_map<uint16_t, MessageFactory::Creator> MessageFactory::creators_;
std::unordered_map<uint16_t, std::string> MessageFactory::message_type_names_;
std::unordered_map<uint8_t, std::string> MessageFactory::sub_message_type_names_;
std::mutex MessageFactory::factory_mutex_;

void MessageFactory::RegisterCreator(uint16_t message_id, Creator creator) {
    std::lock_guard<std::mutex> lock(factory_mutex_);
    creators_[message_id] = std::move(creator);
}

Message::Ptr MessageFactory::CreateMessage(uint8_t message_type, uint16_t message_id, uint8_t sub_message_id) {
    std::lock_guard<std::mutex> lock(factory_mutex_);
    
    auto it = creators_.find(message_id);
    if (it != creators_.end()) {
        return it->second();
    }
    
    // 默认创建基础消息
    switch (message_type) {
        case 1: // REQUEST
            return std::make_shared<RequestMessage>(message_id, sub_message_id);
        case 2: // RESPONSE
            return std::make_shared<ResponseMessage>(message_id, sub_message_id);
        case 3: // NOTIFY
            return std::make_shared<NotifyMessage>(message_id, sub_message_id);
        default:
            return std::make_shared<Message>(message_type, message_id, sub_message_id);
    }
}

Message::Ptr MessageFactory::CreateFromBytes(const std::vector<uint8_t>& data) {
    if (data.size() < 3) {
        return nullptr;
    }
    
    uint16_t message_id = static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
    uint8_t sub_message_id = data[2];
    
    auto message = CreateMessage(0, message_id, sub_message_id);
    if (message && message->Deserialize(data)) {
        return message;
    }
    
    return nullptr;
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

// MessageRouter实现
void MessageRouter::RegisterHandler(uint16_t message_id, MessageHandler::Ptr handler) {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    handlers_[message_id] = std::move(handler);
}

void MessageRouter::UnregisterHandler(uint16_t message_id) {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    handlers_.erase(message_id);
}

Message::Ptr MessageRouter::RouteMessage(const Message::Ptr& message) {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    
    auto it = handlers_.find(message->GetMessageId());
    if (it != handlers_.end()) {
        return it->second->HandleMessage(message);
    }
    
    return nullptr;
}

bool MessageRouter::HasHandler(uint16_t message_id) const {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    return handlers_.find(message_id) != handlers_.end();
}

} // namespace perception
