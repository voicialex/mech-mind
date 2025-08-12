#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <mutex>

namespace perception {

/**
 * @brief 消息类型枚举 - 符合充电枪协议
 */
enum class MessageType : uint8_t {
    Request = 0x00,    // 请求
    Response = 0x01,   // 响应
    Notify = 0x02      // 通知
};

/**
 * @brief 消息状态枚举
 */
enum class MessageStatus : uint8_t {
    Pending = 0x00,    // 等待中
    Processing = 0x01, // 处理中
    Completed = 0x02,  // 已完成
    Error = 0x03       // 错误
};

/**
 * @brief 协议帧结构 - 严格按照充电枪协议设计
 * 
 * 协议帧格式：
 * ┌─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
 * │ Magic   │ CRC16   │ MsgType │ MsgID   │ SubMsgID│ Sequence│
 * │ (2字节)  │ (2字节)  │ (1字节)  │ (2字节)  │ (1字节)  │ (2字节)  │
 * ├─────────┴─────────┴─────────┴─────────┴─────────┴─────────┤
 * │ Length  │ Payload                                         │
 * │ (2字节)  │ (变长)                                          │
 * └───────────────────────────────────────────────────────────┘
 */
struct ProtocolFrame {
    uint16_t magic_id;      // 魔术字 (0xBE01)
    uint16_t crc16;         // CRC16校验
    uint8_t message_type;   // 消息类型
    uint16_t message_id;    // 消息ID
    uint8_t sub_message_id; // 子消息ID (阶段码)
    uint16_t sequence;      // 序列号
    uint16_t length;        // 载荷长度
    std::vector<uint8_t> payload; // 载荷数据
    
    static constexpr uint16_t MAGIC_ID = 0xBE01;
    static constexpr size_t HEADER_SIZE = 12;
    static constexpr size_t MIN_FRAME_SIZE = HEADER_SIZE;
    static constexpr size_t MAX_PAYLOAD_SIZE = 65535;
};

/**
 * @brief 消息基类 - 基于协议帧
 */
class Message {
public:
    using Ptr = std::shared_ptr<Message>;
    using Handler = std::function<void(const Message::Ptr&)>;
    
    Message();
    Message(uint8_t message_type, uint16_t message_id, uint8_t sub_message_id = 0);
    virtual ~Message() = default;
    
    // 禁用拷贝
    Message(const Message&) = delete;
    Message& operator=(const Message&) = delete;
    
    /**
     * @brief 序列化消息为协议帧
     * @return 序列化后的字节数组
     */
    virtual std::vector<uint8_t> Serialize() const;
    
    /**
     * @brief 从协议帧反序列化消息
     * @param data 字节数组
     * @return 是否反序列化成功
     */
    virtual bool Deserialize(const std::vector<uint8_t>& data);
    
    /**
     * @brief 验证协议帧完整性
     * @param data 字节数组
     * @return 是否有效
     */
    static bool ValidateFrame(const std::vector<uint8_t>& data);
    
    /**
     * @brief 计算CRC16校验值 (CRC16-IBM)
     * @param data 数据
     * @return CRC16值
     */
    static uint16_t CalculateCRC16(const std::vector<uint8_t>& data);
    
    /**
     * @brief 验证CRC16校验值
     * @param data 数据
     * @param crc16 CRC16值
     * @return 是否验证通过
     */
    static bool VerifyCRC16(const std::vector<uint8_t>& data, uint16_t crc16);
    
    /**
     * @brief 解析协议帧
     * @param data 字节数组
     * @return 协议帧结构
     */
    static ProtocolFrame ParseFrame(const std::vector<uint8_t>& data);
    
    /**
     * @brief 构建协议帧
     * @param frame 协议帧结构
     * @return 字节数组
     */
    static std::vector<uint8_t> BuildFrame(const ProtocolFrame& frame);
    
    // Getters
    uint8_t GetMessageType() const { return frame_.message_type; }
    uint16_t GetMessageId() const { return frame_.message_id; }
    uint8_t GetSubMessageId() const { return frame_.sub_message_id; }
    uint16_t GetSequence() const { return frame_.sequence; }
    uint16_t GetLength() const { return frame_.length; }
    const std::vector<uint8_t>& GetPayload() const { return frame_.payload; }
    MessageStatus GetStatus() const { return status_; }
    const ProtocolFrame& GetFrame() const { return frame_; }
    
    // Setters
    void SetMessageType(uint8_t type) { frame_.message_type = type; }
    void SetMessageId(uint16_t id) { frame_.message_id = id; }
    void SetSubMessageId(uint8_t sub_id) { frame_.sub_message_id = sub_id; }
    void SetSequence(uint16_t seq) { frame_.sequence = seq; }
    void SetPayload(const std::vector<uint8_t>& payload);
    void SetStatus(MessageStatus status) { status_ = status; }
    
    /**
     * @brief 获取消息类型字符串
     */
    std::string GetMessageTypeString() const;
    
    /**
     * @brief 获取子消息类型字符串
     */
    std::string GetSubMessageTypeString() const;
    
    /**
     * @brief 转换为字符串表示
     */
    virtual std::string ToString() const;
    
    /**
     * @brief 转换为十六进制字符串
     */
    std::string ToHexString() const;

protected:
    ProtocolFrame frame_;
    MessageStatus status_;
    std::chrono::steady_clock::time_point timestamp_;
};

/**
 * @brief 请求消息类
 */
class RequestMessage : public Message {
public:
    RequestMessage(uint16_t message_id, uint8_t sub_message_id = 0);
    
    /**
     * @brief 设置请求参数
     * @param params 参数数据
     */
    void SetParameters(const std::vector<uint8_t>& params);
    
    /**
     * @brief 获取请求参数
     * @return 参数数据
     */
    std::vector<uint8_t> GetParameters() const;
    
    std::string ToString() const override;
};

/**
 * @brief 响应消息类
 */
class ResponseMessage : public Message {
public:
    ResponseMessage(uint16_t message_id, uint8_t sub_message_id = 0);
    
    /**
     * @brief 设置响应结果
     * @param result 结果数据
     */
    void SetResult(const std::vector<uint8_t>& result);
    
    /**
     * @brief 获取响应结果
     * @return 结果数据
     */
    std::vector<uint8_t> GetResult() const;
    
    /**
     * @brief 设置错误码
     * @param error_code 错误码
     */
    void SetErrorCode(uint16_t error_code);
    
    /**
     * @brief 获取错误码
     * @return 错误码
     */
    uint16_t GetErrorCode() const;
    
    std::string ToString() const override;

private:
    uint16_t error_code_;
};

/**
 * @brief 通知消息类
 */
class NotifyMessage : public Message {
public:
    NotifyMessage(uint16_t message_id, uint8_t sub_message_id = 0);
    
    /**
     * @brief 设置通知数据
     * @param data 通知数据
     */
    void SetNotificationData(const std::vector<uint8_t>& data);
    
    /**
     * @brief 获取通知数据
     * @return 通知数据
     */
    std::vector<uint8_t> GetNotificationData() const;
    
    std::string ToString() const override;
};

/**
 * @brief 消息工厂类
 */
class MessageFactory {
public:
    using Creator = std::function<Message::Ptr()>;
    
    /**
     * @brief 注册消息创建器
     * @param message_id 消息ID
     * @param creator 创建器函数
     */
    static void RegisterCreator(uint16_t message_id, Creator creator);
    
    /**
     * @brief 创建消息
     * @param message_type 消息类型
     * @param message_id 消息ID
     * @param sub_message_id 子消息ID
     * @return 消息指针
     */
    static Message::Ptr CreateMessage(uint8_t message_type, uint16_t message_id, uint8_t sub_message_id = 0);
    
    /**
     * @brief 从字节数组创建消息
     * @param data 字节数组
     * @return 消息指针
     */
    static Message::Ptr CreateFromBytes(const std::vector<uint8_t>& data);
    
    /**
     * @brief 获取消息类型名称
     * @param message_id 消息ID
     * @return 消息类型名称
     */
    static std::string GetMessageTypeName(uint16_t message_id);
    
    /**
     * @brief 获取子消息类型名称
     * @param sub_message_id 子消息ID
     * @return 子消息类型名称
     */
    static std::string GetSubMessageTypeName(uint8_t sub_message_id);

private:
    static std::unordered_map<uint16_t, Creator> creators_;
    static std::unordered_map<uint16_t, std::string> message_type_names_;
    static std::unordered_map<uint8_t, std::string> sub_message_type_names_;
    static std::mutex factory_mutex_;
};

/**
 * @brief 消息处理器基类
 */
class MessageHandler {
public:
    using Ptr = std::shared_ptr<MessageHandler>;
    
    virtual ~MessageHandler() = default;
    
    /**
     * @brief 处理消息
     * @param message 消息
     * @return 响应消息
     */
    virtual Message::Ptr HandleMessage(const Message::Ptr& message) = 0;
    
    /**
     * @brief 获取支持的消息ID
     * @return 消息ID列表
     */
    virtual std::vector<uint16_t> GetSupportedMessageIds() const = 0;
};

/**
 * @brief 消息路由器
 */
class MessageRouter {
public:
    using Ptr = std::shared_ptr<MessageRouter>;
    
    /**
     * @brief 注册消息处理器
     * @param message_id 消息ID
     * @param handler 处理器
     */
    void RegisterHandler(uint16_t message_id, MessageHandler::Ptr handler);
    
    /**
     * @brief 注销消息处理器
     * @param message_id 消息ID
     */
    void UnregisterHandler(uint16_t message_id);
    
    /**
     * @brief 路由消息
     * @param message 消息
     * @return 响应消息
     */
    Message::Ptr RouteMessage(const Message::Ptr& message);
    
    /**
     * @brief 检查是否有处理器
     * @param message_id 消息ID
     * @return 是否有处理器
     */
    bool HasHandler(uint16_t message_id) const;

private:
    std::unordered_map<uint16_t, MessageHandler::Ptr> handlers_;
    mutable std::mutex handlers_mutex_;
};

} // namespace perception
