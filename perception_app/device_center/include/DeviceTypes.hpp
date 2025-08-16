#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace device_center {

/**
 * @brief 设备状态枚举
 */
enum class DeviceStatus : uint8_t {
    Offline = 0x00,      // 离线
    Online = 0x01,       // 在线
    Busy = 0x02,         // 忙碌
    Error = 0x03,        // 错误
    Maintenance = 0x04   // 维护中
};

/**
 * @brief 设备能力枚举
 */
enum class DeviceCapability : uint8_t {
    Read = 0x01,         // 读取数据
    Write = 0x02,        // 写入数据
    Control = 0x03,      // 控制
    Configure = 0x04,    // 配置
    Status = 0x05,       // 状态查询
    Calibrate = 0x06,    // 校准
    Capture = 0x07,      // 采集
    Process = 0x08       // 处理
};

/**
 * @brief 设备信息结构
 */
struct DeviceInfo {
    std::string device_id;           // 设备ID
    std::string device_name;         // 设备名称
    std::string device_model;        // 设备型号
    std::string device_version;      // 设备版本
    std::string device_serial;       // 设备序列号
    std::string device_ip;           // 设备IP地址
    uint16_t device_port;            // 设备端口
    std::vector<DeviceCapability> capabilities; // 设备能力
    DeviceStatus status;             // 设备状态
    std::string status_message;      // 状态消息
    uint64_t last_heartbeat;         // 最后心跳时间
    nlohmann::json config;           // 设备配置
    nlohmann::json metadata;         // 设备元数据
};

/**
 * @brief 设备状态信息
 */
struct DeviceStatusInfo {
    std::string device_id;           // 设备ID
    DeviceStatus status;             // 设备状态
    std::string status_message;      // 状态消息
    uint64_t timestamp;              // 时间戳
    nlohmann::json status_data;      // 状态数据
    float cpu_usage;                 // CPU使用率
    float memory_usage;              // 内存使用率
    float temperature;               // 温度
    std::map<std::string, float> sensors; // 传感器数据
};

/**
 * @brief 设备命令结构
 */
struct DeviceCommand {
    std::string device_id;           // 设备ID
    std::string command_id;          // 命令ID
    std::string command_type;        // 命令类型
    nlohmann::json parameters;       // 命令参数
    uint64_t timestamp;              // 时间戳
    uint32_t timeout_ms;             // 超时时间
    bool require_response;           // 是否需要响应
};

/**
 * @brief 设备响应结构
 */
struct DeviceResponse {
    std::string device_id;           // 设备ID
    std::string command_id;          // 命令ID
    bool success;                    // 是否成功
    std::string message;             // 响应消息
    nlohmann::json data;             // 响应数据
    uint64_t timestamp;              // 时间戳
    uint16_t error_code;             // 错误码
};

/**
 * @brief 设备数据结构
 */
struct DeviceData {
    std::string device_id;           // 设备ID
    std::string data_type;           // 数据类型
    uint64_t timestamp;              // 时间戳
    nlohmann::json data;             // 数据内容
    std::string data_format;         // 数据格式
    uint32_t data_size;              // 数据大小
    bool compressed;                 // 是否压缩
};

/**
 * @brief 设备注册请求
 */
struct DeviceRegisterRequest {
    std::string device_id;           // 设备ID
    std::string device_name;         // 设备名称
    std::string device_model;        // 设备型号
    std::string device_version;      // 设备版本
    std::vector<DeviceCapability> capabilities; // 设备能力
    nlohmann::json config;           // 设备配置
    std::string client_id;           // 客户端ID
};

/**
 * @brief 设备注册响应
 */
struct DeviceRegisterResponse {
    bool success;                    // 是否成功
    std::string message;             // 响应消息
    std::string device_id;           // 设备ID
    uint16_t error_code;             // 错误码
    nlohmann::json server_config;    // 服务器配置
};

/**
 * @brief 设备发现请求
 */
struct DeviceDiscoveryRequest {
    std::string client_id;           // 客户端ID
    std::string device_name;         // 设备名称（可选）
    bool include_offline;            // 是否包含离线设备
};

/**
 * @brief 设备发现响应
 */
struct DeviceDiscoveryResponse {
    bool success;                    // 是否成功
    std::string message;             // 响应消息
    std::vector<DeviceInfo> devices; // 设备列表
    uint16_t error_code;             // 错误码
};

/**
 * @brief 设备控制请求
 */
struct DeviceControlRequest {
    std::string device_id;           // 设备ID
    std::string command_type;        // 命令类型
    nlohmann::json parameters;       // 命令参数
    uint32_t timeout_ms;             // 超时时间
    bool require_response;           // 是否需要响应
};

/**
 * @brief 设备控制响应
 */
struct DeviceControlResponse {
    bool success;                    // 是否成功
    std::string message;             // 响应消息
    std::string device_id;           // 设备ID
    std::string command_type;        // 命令类型
    nlohmann::json result;           // 执行结果
    uint16_t error_code;             // 错误码
};

/**
 * @brief 设备处理器接口
 */
class DeviceHandler {
public:
    using Ptr = std::shared_ptr<DeviceHandler>;
    
    virtual ~DeviceHandler() = default;
    
    /**
     * @brief 处理设备命令
     * @param command 设备命令
     * @return 设备响应
     */
    virtual DeviceResponse HandleCommand(const DeviceCommand& command) = 0;
    
    /**
     * @brief 获取设备状态
     * @param device_id 设备ID
     * @return 设备状态
     */
    virtual DeviceStatusInfo GetDeviceStatus(const std::string& device_id) = 0;
    
    /**
     * @brief 获取设备信息
     * @param device_id 设备ID
     * @return 设备信息
     */
    virtual DeviceInfo GetDeviceInfo(const std::string& device_id) = 0;
};

/**
 * @brief 设备事件回调接口
 */
class DeviceEventHandler {
public:
    using Ptr = std::shared_ptr<DeviceEventHandler>;
    
    virtual ~DeviceEventHandler() = default;
    
    /**
     * @brief 设备状态变化事件
     * @param device_id 设备ID
     * @param old_status 旧状态
     * @param new_status 新状态
     */
    virtual void OnDeviceStatusChanged(const std::string& device_id, 
                                     DeviceStatus old_status, 
                                     DeviceStatus new_status) = 0;
    
    /**
     * @brief 设备数据接收事件
     * @param device_data 设备数据
     */
    virtual void OnDeviceDataReceived(const DeviceData& device_data) = 0;
    
    /**
     * @brief 设备错误事件
     * @param device_id 设备ID
     * @param error_code 错误码
     * @param error_message 错误消息
     */
    virtual void OnDeviceError(const std::string& device_id, 
                              uint16_t error_code, 
                              const std::string& error_message) = 0;
};

/**
 * @brief 工具函数
 */
namespace DeviceUtils {
    
    /**
     * @brief 设备状态转字符串
     * @param status 设备状态
     * @return 状态字符串
     */
    std::string StatusToString(DeviceStatus status);
    
    /**
     * @brief 字符串转设备状态
     * @param status_str 状态字符串
     * @return 设备状态
     */
    DeviceStatus StringToStatus(const std::string& status_str);
    
    /**
     * @brief 设备能力转字符串
     * @param capability 设备能力
     * @return 能力字符串
     */
    std::string CapabilityToString(DeviceCapability capability);
    
    /**
     * @brief 字符串转设备能力
     * @param capability_str 能力字符串
     * @return 设备能力
     */
    DeviceCapability StringToCapability(const std::string& capability_str);
    
    /**
     * @brief 生成设备ID
     * @param device_name 设备名称
     * @return 设备ID
     */
    std::string GenerateDeviceId(const std::string& device_name);
    
    /**
     * @brief 验证设备ID格式
     * @param device_id 设备ID
     * @return 是否有效
     */
    bool ValidateDeviceId(const std::string& device_id);
    
    /**
     * @brief 获取当前时间戳
     * @return 时间戳
     */
    uint64_t GetCurrentTimestamp();
    
    /**
     * @brief 时间戳转字符串
     * @param timestamp 时间戳
     * @return 时间字符串
     */
    std::string TimestampToString(uint64_t timestamp);
    
    /**
     * @brief 字符串转时间戳
     * @param time_str 时间字符串
     * @return 时间戳
     */
    uint64_t StringToTimestamp(const std::string& time_str);
}

} // namespace device_center
