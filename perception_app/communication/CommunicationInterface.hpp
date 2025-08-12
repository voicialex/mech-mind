#pragma once

#include "CommunicationManager.hpp"
#include "message/MessageProtocol.hpp"
#include <memory>
#include <functional>
#include <string>

namespace perception {

/**
 * @brief 通信接口 - 为runtime提供统一的通信能力
 * 
 * 设计原则：
 * - 只提供通信功能，不耦合业务逻辑
 * - 提供简单的消息发送和接收接口
 * - 支持同步和异步通信模式
 * - 自动处理连接管理和错误恢复
 */
class CommunicationInterface {
public:
    using MessageCallback = std::function<void(const Message::Ptr&)>;
    using ConnectionCallback = std::function<void(const std::string&, bool)>;
    using ErrorCallback = std::function<void(const std::string&, uint16_t)>;

    /**
     * @brief 通信接口配置
     */
    struct Config {
        std::string local_service_id = "perception_runtime";
        std::string local_service_name = "Perception Runtime";
        std::string local_address = "0.0.0.0";
        uint16_t local_port = 8080;
        uint16_t discovery_port = 8081;
        uint32_t max_connections = 50;
        uint32_t buffer_size = 8192;
        uint32_t heartbeat_interval = 5000; // ms
        uint32_t connection_timeout = 30000; // ms
        bool enable_auto_reconnect = true;
        uint32_t max_reconnect_attempts = 5;
        uint32_t reconnect_delay = 1000; // ms
    };

public:
    explicit CommunicationInterface(const Config& config);
    CommunicationInterface(); // 默认构造函数
    ~CommunicationInterface();

    // 禁用拷贝
    CommunicationInterface(const CommunicationInterface&) = delete;
    CommunicationInterface& operator=(const CommunicationInterface&) = delete;

    /**
     * @brief 初始化通信接口
     * @return 是否初始化成功
     */
    bool Initialize();

    /**
     * @brief 启动通信服务
     * @return 是否启动成功
     */
    bool Start();

    /**
     * @brief 停止通信服务
     */
    void Stop();

    /**
     * @brief 清理资源
     */
    void Cleanup();

    /**
     * @brief 注册消息回调
     * @param callback 消息处理回调函数
     */
    void RegisterMessageCallback(MessageCallback callback);

    /**
     * @brief 注册连接状态回调
     * @param callback 连接状态回调函数
     */
    void RegisterConnectionCallback(ConnectionCallback callback);

    /**
     * @brief 注册错误回调
     * @param callback 错误处理回调函数
     */
    void RegisterErrorCallback(ErrorCallback callback);

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
     * @brief 发送消息
     * @param target_id 目标服务ID
     * @param message 消息对象
     * @return 是否发送成功
     */
    bool SendMessage(const std::string& target_id, const Message::Ptr& message);

    /**
     * @brief 发送消息并等待响应
     * @param target_id 目标服务ID
     * @param message 消息对象
     * @param timeout_ms 超时时间(毫秒)
     * @return 响应消息
     */
    Message::Ptr SendRequest(const std::string& target_id, 
                           const Message::Ptr& message,
                           uint32_t timeout_ms = 5000);

    /**
     * @brief 广播消息
     * @param message 消息对象
     * @param service_name 目标服务名称（可选）
     */
    void BroadcastMessage(const Message::Ptr& message, const std::string& service_name = "");

    /**
     * @brief 发现服务
     * @param service_name 服务名称（可选）
     * @return 服务列表
     */
    std::vector<CommunicationManager::ServiceInfo> DiscoverServices(const std::string& service_name = "");

    /**
     * @brief 检查服务是否在线
     * @param service_id 服务ID
     * @return 是否在线
     */
    bool IsServiceOnline(const std::string& service_id) const;

    /**
     * @brief 获取连接状态
     * @param service_id 服务ID
     * @return 连接信息
     */
    CommunicationManager::ConnectionInfo GetConnectionInfo(const std::string& service_id) const;

    /**
     * @brief 获取所有连接
     * @return 连接信息列表
     */
    std::vector<CommunicationManager::ConnectionInfo> GetAllConnections() const;

    /**
     * @brief 获取本地服务信息
     * @return 本地服务信息
     */
    CommunicationManager::ServiceInfo GetLocalServiceInfo() const;

    /**
     * @brief 获取统计信息
     * @return 统计信息JSON
     */
    std::string GetStatistics() const;

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

private:
    // 私有方法
    void InitializeCommunicationManager();
    void HandleMessageReceived(const std::string& service_id, const std::vector<uint8_t>& data);
    void HandleConnectionStatus(const std::string& service_id, bool connected);
    void HandleError(const std::string& service_id, const asio::error_code& error);

private:
    Config config_;
    std::unique_ptr<CommunicationManager> comm_manager_;
    
    // 回调函数
    MessageCallback message_callback_;
    ConnectionCallback connection_callback_;
    ErrorCallback error_callback_;
    
    // 状态管理
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    
    // 消息工厂
    std::unique_ptr<MessageFactory> message_factory_;
};

/**
 * @brief 通信接口管理器 - 管理多个通信接口实例
 */
class CommunicationInterfaceManager {
public:
    using Ptr = std::shared_ptr<CommunicationInterfaceManager>;
    
    explicit CommunicationInterfaceManager();
    ~CommunicationInterfaceManager();

    // 禁用拷贝
    CommunicationInterfaceManager(const CommunicationInterfaceManager&) = delete;
    CommunicationInterfaceManager& operator=(const CommunicationInterfaceManager&) = delete;

    /**
     * @brief 创建通信接口
     * @param name 接口名称
     * @param config 配置
     * @return 是否创建成功
     */
    bool CreateInterface(const std::string& name, const CommunicationInterface::Config& config);

    /**
     * @brief 获取通信接口
     * @param name 接口名称
     * @return 通信接口指针
     */
    std::shared_ptr<CommunicationInterface> GetInterface(const std::string& name);

    /**
     * @brief 移除通信接口
     * @param name 接口名称
     */
    void RemoveInterface(const std::string& name);

    /**
     * @brief 启动所有接口
     * @return 是否全部启动成功
     */
    bool StartAll();

    /**
     * @brief 停止所有接口
     */
    void StopAll();

    /**
     * @brief 获取所有接口状态
     * @return 接口状态列表
     */
    std::vector<std::pair<std::string, bool>> GetAllInterfaceStatus() const;

    /**
     * @brief 清理所有资源
     */
    void Cleanup();

private:
    std::unordered_map<std::string, std::shared_ptr<CommunicationInterface>> interfaces_;
    mutable std::mutex interfaces_mutex_;
};

} // namespace perception
