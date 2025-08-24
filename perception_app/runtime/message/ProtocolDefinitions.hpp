#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector> // Added for std::vector

// 宏函数：将标识符转换为字符串
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// 宏函数：将枚举值转换为字符串
#define ENUM_TO_STRING(enum_val) TOSTRING(enum_val)

namespace perception {

/**
 * @brief 协议常量定义
 */
namespace ProtocolConstants {
    static constexpr uint16_t MAGIC_ID = 0xBE01;        // 魔术字
    static constexpr size_t HEADER_SIZE = 12;           // 头部大小
    static constexpr size_t MIN_FRAME_SIZE = HEADER_SIZE; // 最小帧大小
    static constexpr size_t MAX_PAYLOAD_SIZE = 65535;   // 最大负载大小
    
    // 字段偏移量
    static constexpr size_t MAGIC_OFFSET = 0;           // 魔术字偏移 (2字节)
    static constexpr size_t CRC16_OFFSET = 2;           // CRC16偏移 (2字节)
    static constexpr size_t MSG_TYPE_OFFSET = 4;        // 消息类型偏移 (1字节)
    static constexpr size_t MSG_ID_OFFSET = 5;          // 消息ID偏移 (2字节)
    static constexpr size_t SUB_MSG_ID_OFFSET = 7;      // 子消息ID偏移 (1字节)
    static constexpr size_t SEQUENCE_OFFSET = 8;        // 序列号偏移 (2字节)
    static constexpr size_t LENGTH_OFFSET = 10;         // 长度偏移 (2字节)
    static constexpr size_t PAYLOAD_OFFSET = 12;        // 负载偏移 (变长)
}

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
 * @brief 消息ID定义 - 符合充电枪协议规范
 */
namespace MessageIds {
    // 系统消息 (0x0001-0x00FF)
    static constexpr uint16_t HEARTBEAT_REQUEST = 0x0001;  // 心跳请求（服务器发起）
    static constexpr uint16_t HEARTBEAT_RESPONSE = 0x0002; // 心跳响应（客户端回复）
    static constexpr uint16_t SERVICE_DISCOVERY = 0x0003;
    static constexpr uint16_t CONNECTION_REQUEST = 0x0004;
    static constexpr uint16_t CONNECTION_RESPONSE = 0x0005;
    
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
    
    // 使用ProtocolConstants中的常量
    static constexpr uint16_t MAGIC_ID = ProtocolConstants::MAGIC_ID;
    static constexpr size_t HEADER_SIZE = ProtocolConstants::HEADER_SIZE;
    static constexpr size_t MIN_FRAME_SIZE = ProtocolConstants::MIN_FRAME_SIZE;
    static constexpr size_t MAX_PAYLOAD_SIZE = ProtocolConstants::MAX_PAYLOAD_SIZE;
};

/**
 * @brief 协议工具类
 */
class ProtocolUtils {
public:
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
     * @brief 验证消息格式
     * @param data 字节数组
     * @return 是否有效
     */
    static bool ValidateMessage(const std::vector<uint8_t>& data);
};

} // namespace perception
