#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <mutex>
#include <unordered_map>

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
 * @brief 消息ID定义 - 符合充电枪协议规范
 */
namespace MessageIds {
    // 系统消息 (0x0001-0x00FF)
    static constexpr uint16_t HEARTBEAT = 0x0001;
    static constexpr uint16_t SERVICE_DISCOVERY = 0x0002;
    static constexpr uint16_t CONNECTION_REQUEST = 0x0003;
    static constexpr uint16_t CONNECTION_RESPONSE = 0x0004;
    
    // 充电枪操作消息 (0x0100-0x01FF)
    static constexpr uint16_t START_CHARGING = 0x0100;
    static constexpr uint16_t STOP_CHARGING = 0x0101;
    static constexpr uint16_t EMERGENCY_STOP = 0x0102;
    
    // 设备控制消息 (0x0200-0x02FF)
    static constexpr uint16_t DEVICE_CONTROL = 0x0200;
    static constexpr uint16_t DEVICE_STATUS = 0x0201;
    static constexpr uint16_t DEVICE_CONFIG = 0x0202;
}

/**
 * @brief 子消息ID定义 (阶段码) - 符合充电枪协议规范
 */
namespace SubMessageIds {
    // 系统阶段 (0x00-0x0F)
    static constexpr uint8_t IDLE = 0x00;
    static constexpr uint8_t INITIALIZING = 0x01;
    static constexpr uint8_t CONNECTING = 0x02;
    static constexpr uint8_t READY = 0x03;
    static constexpr uint8_t ERROR = 0x0F;
    
    // 充电枪操作阶段 (0x10-0x1F)
    static constexpr uint8_t DEVICE_SELF_CHECK = 0x10;
    static constexpr uint8_t COVER_OPERATION = 0x11;
    static constexpr uint8_t TARGET_DETECTION = 0x12;
    static constexpr uint8_t PATH_PLANNING = 0x13;
    static constexpr uint8_t INSERTION = 0x14;
    static constexpr uint8_t REMOVAL = 0x15;
    static constexpr uint8_t CONNECTION_VERIFICATION = 0x16;
    static constexpr uint8_t COMPLETED = 0x17;
}

/**
 * @brief 错误码定义 - 符合充电枪协议规范
 */
namespace ErrorCodes {
    static constexpr uint16_t SUCCESS = 0x0000;
    static constexpr uint16_t GENERAL_ERROR = 0x0001;
    static constexpr uint16_t INVALID_PARAMETER = 0x0002;
    static constexpr uint16_t TIMEOUT = 0x0003;
    static constexpr uint16_t NOT_FOUND = 0x0004;
    static constexpr uint16_t ALREADY_EXISTS = 0x0005;
    static constexpr uint16_t PERMISSION_DENIED = 0x0006;
    static constexpr uint16_t RESOURCE_UNAVAILABLE = 0x0007;
    
    // 充电枪操作错误 (0x0100-0x01FF)
    static constexpr uint16_t DEVICE_DISCONNECTED = 0x0100;
    static constexpr uint16_t SELF_CHECK_FAILED = 0x0101;
    static constexpr uint16_t TARGET_RECOGNITION_FAILED = 0x0102;
    static constexpr uint16_t COVER_OPERATION_FAILED = 0x0103;
    static constexpr uint16_t PATH_PLANNING_FAILED = 0x0104;
    static constexpr uint16_t INSERTION_FAILED = 0x0105;
    static constexpr uint16_t REMOVAL_FAILED = 0x0106;
    static constexpr uint16_t CONNECTION_FAILED = 0x0107;
}

/**
 * @brief 基础消息接口
 */
class IMessage {
public:
    using Ptr = std::shared_ptr<IMessage>;
    
    virtual ~IMessage() = default;
    
    /**
     * @brief 获取消息类型
     * @return 消息类型
     */
    virtual MessageType GetType() const = 0;
    
    /**
     * @brief 获取消息ID
     * @return 消息ID
     */
    virtual uint16_t GetMessageId() const = 0;
    
    /**
     * @brief 获取子消息ID
     * @return 子消息ID
     */
    virtual uint8_t GetSubMessageId() const = 0;
    
    /**
     * @brief 获取序列号
     * @return 序列号
     */
    virtual uint16_t GetSequence() const = 0;
    
    /**
     * @brief 获取时间戳
     * @return 时间戳
     */
    virtual uint64_t GetTimestamp() const = 0;
    
    /**
     * @brief 获取消息状态
     * @return 消息状态
     */
    virtual MessageStatus GetStatus() const = 0;
    
    /**
     * @brief 设置消息状态
     * @param status 消息状态
     */
    virtual void SetStatus(MessageStatus status) = 0;
    
    /**
     * @brief 序列化消息
     * @return 序列化后的字节数组
     */
    virtual std::vector<uint8_t> Serialize() const = 0;
    
    /**
     * @brief 反序列化消息
     * @param data 字节数组
     * @return 是否反序列化成功
     */
    virtual bool Deserialize(const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief 验证消息完整性
     * @return 是否有效
     */
    virtual bool Validate() const = 0;
    
    /**
     * @brief 转换为字符串表示
     * @return 字符串表示
     */
    virtual std::string ToString() const = 0;
    
    /**
     * @brief 获取协议帧
     * @return 协议帧结构
     */
    virtual const ProtocolFrame& GetFrame() const = 0;
    
    /**
     * @brief 设置协议帧
     * @param frame 协议帧结构
     */
    virtual void SetFrame(const ProtocolFrame& frame) = 0;
};

/**
 * @brief 消息工厂类
 */
class MessageFactory {
public:
    using Creator = std::function<IMessage::Ptr()>;
    
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
    static IMessage::Ptr CreateMessage(MessageType message_type, 
                                     uint16_t message_id, 
                                     uint8_t sub_message_id = 0);
    
    /**
     * @brief 从字节数组创建消息
     * @param data 字节数组
     * @return 消息指针
     */
    static IMessage::Ptr CreateFromBytes(const std::vector<uint8_t>& data);
    
    /**
     * @brief 验证消息格式
     * @param data 字节数组
     * @return 是否有效
     */
    static bool ValidateMessage(const std::vector<uint8_t>& data);
    
    /**
     * @brief 创建心跳消息
     * @return 心跳消息的字节数组
     */
    static std::vector<uint8_t> CreateHeartbeatMessage();
    
    /**
     * @brief 计算CRC16校验值 (CRC16-IBM)
     * @param data 数据
     * @return CRC16值
     */
    static uint16_t CalculateCRC16(const std::vector<uint8_t>& data);
    static uint16_t CalculateCRC16(const uint8_t* data, size_t length);
    
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
    
    /**
     * @brief 获取错误码描述
     * @param error_code 错误码
     * @return 错误码描述
     */
    static std::string GetErrorCodeDescription(uint16_t error_code);
    
    // 初始化函数
    static void InitializeMessageTypeNames();
    static void InitializeSubMessageTypeNames();
    static void InitializeErrorCodeDescriptions();

private:
    static std::unordered_map<uint16_t, Creator> creators_;
    static std::unordered_map<uint16_t, std::string> message_type_names_;
    static std::unordered_map<uint8_t, std::string> sub_message_type_names_;
    static std::unordered_map<uint16_t, std::string> error_code_descriptions_;
    static std::mutex factory_mutex_;
};

/**
 * @brief 消息处理器接口
 */
class IMessageHandler {
public:
    using Ptr = std::shared_ptr<IMessageHandler>;
    
    virtual ~IMessageHandler() = default;
    
    /**
     * @brief 处理消息
     * @param message 消息
     * @return 响应消息
     */
    virtual IMessage::Ptr HandleMessage(const IMessage::Ptr& message) = 0;
    
    /**
     * @brief 获取支持的消息ID
     * @return 支持的消息ID列表
     */
    virtual std::vector<uint16_t> GetSupportedMessageIds() const = 0;
    
    /**
     * @brief 检查是否支持消息
     * @param message_id 消息ID
     * @return 是否支持
     */
    virtual bool SupportsMessage(uint16_t message_id) const = 0;
};

/**
 * @brief 消息路由器接口
 */
class IMessageRouter {
public:
    using Ptr = std::shared_ptr<IMessageRouter>;
    
    virtual ~IMessageRouter() = default;
    
    /**
     * @brief 注册消息处理器
     * @param message_id 消息ID
     * @param handler 处理器
     */
    virtual void RegisterHandler(uint16_t message_id, IMessageHandler::Ptr handler) = 0;
    
    /**
     * @brief 注销消息处理器
     * @param message_id 消息ID
     */
    virtual void UnregisterHandler(uint16_t message_id) = 0;
    
    /**
     * @brief 路由消息
     * @param message 消息
     * @return 响应消息
     */
    virtual IMessage::Ptr RouteMessage(const IMessage::Ptr& message) = 0;
    
    /**
     * @brief 检查是否有处理器
     * @param message_id 消息ID
     * @return 是否有处理器
     */
    virtual bool HasHandler(uint16_t message_id) const = 0;
    
    /**
     * @brief 获取处理器数量
     * @return 处理器数量
     */
    virtual size_t GetHandlerCount() const = 0;
};

/**
 * @brief 创建消息路由器实例的工厂函数
 * @return 消息路由器指针
 */
IMessageRouter::Ptr CreateMessageRouter();

} // namespace perception
