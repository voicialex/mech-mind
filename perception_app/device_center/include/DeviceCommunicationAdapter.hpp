#pragma once

#include "DeviceTypes.hpp"
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

// 前向声明，避免直接依赖
namespace perception {
    class CommunicationInterface;
    class Message;
}

namespace device_center {

/**
 * @brief 设备通信适配器 - 统一通信层
 * 
 * 设计原则：
 * - 作为 device_center 与 perception_app/communication 之间的适配层
 * - 提供统一的设备通信接口
 * - 隐藏底层通信细节
 * - 支持服务器和客户端模式
 */
class DeviceCommunicationAdapter {
public:
    /**
     * @brief 通信模式
     */
    enum class Mode {
        Server,  // 服务器模式
        Client   // 客户端模式
    };

    /**
     * @brief 消息类型
     */
    enum class MessageType {
        DeviceRegister,      // 设备注册
        DeviceUnregister,    // 设备注销
        DeviceStatus,        // 设备状态
        DeviceCommand,       // 设备命令
        DeviceResponse,      // 设备响应
        DeviceData,          // 设备数据
        Heartbeat,           // 心跳
        Discovery,           // 服务发现
        Error               // 错误
    };

    /**
     * @brief 统一消息结构
     */
    struct DeviceMessage {
        MessageType type;
        std::string source_id;
        std::string target_id;
        uint64_t timestamp;
        nlohmann::json data;
        std::string message_id;
        
        DeviceMessage() : type(MessageType::Error), timestamp(0) {}
    };

    /**
     * @brief 适配器配置
     */
    struct Config {
        Mode mode = Mode::Client;
        std::string service_id;
        std::string service_name;
        std::string local_address = "0.0.0.0";
        uint16_t local_port = 0;
        uint16_t discovery_port = 0;
        uint32_t heartbeat_interval = 5000; // ms
        uint32_t connection_timeout = 30000; // ms
        bool enable_auto_reconnect = true;
        uint32_t max_reconnect_attempts = 5;
        std::string log_level = "INFO";
    };

    /**
     * @brief 连接状态
     */
    enum class ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Error
    };

    /**
     * @brief 消息回调函数
     */
    using MessageCallback = std::function<void(const DeviceMessage&)>;
    
    /**
     * @brief 连接状态回调函数
     */
    using ConnectionCallback = std::function<void(const std::string&, ConnectionState)>;
    
    /**
     * @brief 错误回调函数
     */
    using ErrorCallback = std::function<void(const std::string&, uint16_t)>;

public:
    explicit DeviceCommunicationAdapter(const Config& config);
    ~DeviceCommunicationAdapter();

    // 禁用拷贝
    DeviceCommunicationAdapter(const DeviceCommunicationAdapter&) = delete;
    DeviceCommunicationAdapter& operator=(const DeviceCommunicationAdapter&) = delete;

    /**
     * @brief 初始化适配器
     * @return 是否初始化成功
     */
    bool Initialize();

    /**
     * @brief 启动适配器
     * @return 是否启动成功
     */
    bool Start();

    /**
     * @brief 停止适配器
     */
    void Stop();

    /**
     * @brief 清理资源
     */
    void Cleanup();

    /**
     * @brief 连接到远程服务
     * @param service_id 服务ID
     * @return 是否连接成功
     */
    bool ConnectToService(const std::string& service_id);

    /**
     * @brief 断开连接
     * @param service_id 服务ID
     */
    void DisconnectFromService(const std::string& service_id);

    /**
     * @brief 发送设备消息
     * @param message 设备消息
     * @return 是否发送成功
     */
    bool SendMessage(const DeviceMessage& message);

    /**
     * @brief 发送请求并等待响应
     * @param request 请求消息
     * @param timeout_ms 超时时间（毫秒）
     * @return 响应消息
     */
    DeviceMessage SendRequest(const DeviceMessage& request, uint32_t timeout_ms = 5000);

    /**
     * @brief 广播消息
     * @param message 消息
     * @param service_name 目标服务名称（可选）
     */
    void BroadcastMessage(const DeviceMessage& message, const std::string& service_name = "");

    /**
     * @brief 发现服务
     * @param service_name 服务名称（可选）
     * @return 发现的服务列表
     */
    std::vector<std::string> DiscoverServices(const std::string& service_name = "");

    /**
     * @brief 注册消息回调
     * @param callback 消息回调函数
     */
    void RegisterMessageCallback(MessageCallback callback);

    /**
     * @brief 注册连接状态回调
     * @param callback 连接状态回调函数
     */
    void RegisterConnectionCallback(ConnectionCallback callback);

    /**
     * @brief 注册错误回调
     * @param callback 错误回调函数
     */
    void RegisterErrorCallback(ErrorCallback callback);

    /**
     * @brief 检查服务是否在线
     * @param service_id 服务ID
     * @return 是否在线
     */
    bool IsServiceOnline(const std::string& service_id) const;

    /**
     * @brief 获取连接状态
     * @param service_id 服务ID
     * @return 连接状态
     */
    ConnectionState GetConnectionState(const std::string& service_id) const;

    /**
     * @brief 获取统计信息
     * @return 统计信息JSON
     */
    nlohmann::json GetStatistics() const;

    /**
     * @brief 检查是否已初始化
     * @return 是否已初始化
     */
    bool IsInitialized() const;

    /**
     * @brief 检查是否正在运行
     * @return 是否正在运行
     */
    bool IsRunning() const;

    /**
     * @brief 获取配置
     * @return 配置
     */
    const Config& GetConfig() const { return config_; }

private:
    // 私有方法
    void InitializeUnderlyingCommunication();
    void OnUnderlyingMessageReceived(const std::shared_ptr<perception::Message>& message);
    void OnUnderlyingConnectionChanged(const std::string& service_id, bool connected);
    void OnUnderlyingError(const std::string& service_id, uint16_t error_code);
    
    // 消息序列化和反序列化
    std::vector<uint8_t> SerializeMessage(const DeviceMessage& message);
    DeviceMessage DeserializeMessage(const std::vector<uint8_t>& data);
    
    // 工具方法
    std::string GenerateMessageId();
    uint64_t GetCurrentTimestamp();

private:
    Config config_;
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    
    // 底层通信接口
    std::unique_ptr<perception::CommunicationInterface> comm_interface_;
    
    // 回调函数
    MessageCallback message_callback_;
    ConnectionCallback connection_callback_;
    ErrorCallback error_callback_;
    
    // 连接状态管理
    mutable std::mutex connections_mutex_;
    std::unordered_map<std::string, ConnectionState> connection_states_;
    
    // 统计信息
    std::atomic<uint32_t> messages_sent_{0};
    std::atomic<uint32_t> messages_received_{0};
    std::atomic<uint32_t> connection_errors_{0};
    uint64_t start_time_{0};
};

} // namespace device_center
