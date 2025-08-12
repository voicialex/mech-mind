#pragma once

#include <asio.hpp>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "../include/Logger.hpp"

namespace perception {

/**
 * @brief 通信管理器 - 基于ASIO的网络通信管理
 * 
 * 职责：
 * 1. 服务发现和注册
 * 2. 连接管理
 * 3. 消息传输
 * 4. 异步通信处理
 * 
 * 设计原则：
 * - 只关注通信功能，不耦合业务逻辑
 * - 提供通用的消息传输接口
 * - 支持多种消息类型和协议
 */
class CommunicationManager {
public:
    using MessageHandler = std::function<void(const std::string&, const std::vector<uint8_t>&)>;
    using ConnectionHandler = std::function<void(const std::string&, bool)>;
    using ErrorHandler = std::function<void(const std::string&, const asio::error_code&)>;

    /**
     * @brief 通信配置
     */
    struct Config {
        std::string local_address = "0.0.0.0";
        uint16_t local_port = 8080;
        uint16_t discovery_port = 8081;
        uint32_t max_connections = 100;
        uint32_t buffer_size = 8192;
        uint32_t heartbeat_interval = 5000; // ms
        uint32_t connection_timeout = 30000; // ms
        bool enable_auto_reconnect = true;
        uint32_t max_reconnect_attempts = 5;
        uint32_t reconnect_delay = 1000; // ms
    };

    /**
     * @brief 服务信息
     */
    struct ServiceInfo {
        std::string service_id;
        std::string service_name;
        std::string address;
        uint16_t port;
        std::string version;
        std::vector<std::string> capabilities;
        uint64_t last_heartbeat;
        bool is_online;
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
     * @brief 连接信息
     */
    struct ConnectionInfo {
        std::string remote_id;
        std::string remote_address;
        uint16_t remote_port;
        ConnectionState state;
        uint64_t last_activity;
        uint32_t message_count;
        uint32_t reconnect_attempts;
    };

    /**
     * @brief 消息传输接口
     */
    struct MessageData {
        std::vector<uint8_t> raw_data;  // 原始消息数据
        std::string source_id;          // 源服务ID
        std::string target_id;          // 目标服务ID
        uint64_t timestamp;             // 时间戳
        uint16_t sequence;              // 序列号
    };

public:
    explicit CommunicationManager(const Config& config);
    CommunicationManager(); // 默认构造函数
    ~CommunicationManager();

    // 禁用拷贝
    CommunicationManager(const CommunicationManager&) = delete;
    CommunicationManager& operator=(const CommunicationManager&) = delete;

    /**
     * @brief 启动通信管理器
     */
    bool Start();

    /**
     * @brief 停止通信管理器
     */
    void Stop();

    /**
     * @brief 注册本地服务
     * @param service_info 服务信息
     * @return 是否注册成功
     */
    bool RegisterService(const ServiceInfo& service_info);

    /**
     * @brief 注销本地服务
     */
    void UnregisterService();

    /**
     * @brief 发现远程服务
     * @param service_name 服务名称（可选）
     * @return 服务列表
     */
    std::vector<ServiceInfo> DiscoverServices(const std::string& service_name = "");

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
     * @brief 发送原始消息数据
     * @param target_id 目标服务ID
     * @param data 消息数据
     * @return 是否发送成功
     */
    bool SendMessage(const std::string& target_id, const std::vector<uint8_t>& data);

    /**
     * @brief 发送消息并等待响应
     * @param target_id 目标服务ID
     * @param data 消息数据
     * @param timeout_ms 超时时间(毫秒)
     * @return 响应数据
     */
    std::vector<uint8_t> SendRequest(const std::string& target_id, 
                                    const std::vector<uint8_t>& data,
                                    uint32_t timeout_ms = 5000);

    /**
     * @brief 广播消息
     * @param data 消息数据
     * @param service_name 目标服务名称（可选）
     */
    void BroadcastMessage(const std::vector<uint8_t>& data, const std::string& service_name = "");

    /**
     * @brief 注册消息接收处理器
     * @param handler 处理器函数
     */
    void RegisterMessageHandler(MessageHandler handler);

    /**
     * @brief 注册连接状态处理器
     * @param handler 处理器函数
     */
    void RegisterConnectionHandler(ConnectionHandler handler);

    /**
     * @brief 注册错误处理器
     * @param handler 处理器函数
     */
    void RegisterErrorHandler(ErrorHandler handler);

    /**
     * @brief 获取连接状态
     * @param service_id 服务ID
     * @return 连接信息
     */
    ConnectionInfo GetConnectionInfo(const std::string& service_id) const;

    /**
     * @brief 获取所有连接
     * @return 连接信息列表
     */
    std::vector<ConnectionInfo> GetAllConnections() const;

    /**
     * @brief 检查服务是否在线
     * @param service_id 服务ID
     * @return 是否在线
     */
    bool IsServiceOnline(const std::string& service_id) const;

    /**
     * @brief 获取本地服务信息
     * @return 本地服务信息
     */
    ServiceInfo GetLocalServiceInfo() const;

    /**
     * @brief 获取统计信息
     * @return 统计信息JSON
     */
    std::string GetStatistics() const;

private:
    // 内部类声明
    class TcpConnection;

    // 私有方法
    void RunIOContext();
    void HandleNewConnection(std::shared_ptr<TcpConnection> connection);
    void HandleConnectionClosed(const std::string& service_id);
    void HandleMessageReceived(const std::string& service_id, 
                             const std::vector<uint8_t>& data);
    void HandleError(const std::string& service_id, const asio::error_code& error);
    void StartHeartbeat();
    void ProcessHeartbeat();
    void CleanupDeadConnections();
    void ScheduleReconnect(const std::string& service_id);

private:
    Config config_;
    std::unique_ptr<asio::io_context> io_context_;
    std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;
    // 简化：暂时不使用这些组件，避免前向声明问题
    // std::unique_ptr<void, DiscoveryDeleter> discovery_;
    // std::unique_ptr<void, MessageProcessorDeleter> message_processor_;
    
    // 服务管理
    ServiceInfo local_service_;
    std::unordered_map<std::string, ServiceInfo> discovered_services_;
    mutable std::mutex services_mutex_;
    
    // 连接管理
    std::unordered_map<std::string, std::shared_ptr<TcpConnection>> connections_;
    mutable std::mutex connections_mutex_;
    
    // 消息处理
    MessageHandler message_handler_;
    std::unordered_map<uint16_t, std::promise<std::vector<uint8_t>>> pending_requests_;
    mutable std::mutex handlers_mutex_;
    
    // 回调处理器
    ConnectionHandler connection_handler_;
    ErrorHandler error_handler_;
    
    // 线程管理
    std::thread io_thread_;
    std::thread heartbeat_thread_;
    std::atomic<bool> running_{false};
    std::atomic<uint16_t> sequence_counter_{1};
    
    // 定时器
    std::unique_ptr<asio::steady_timer> heartbeat_timer_;
    std::unique_ptr<asio::steady_timer> cleanup_timer_;
    std::unique_ptr<asio::steady_timer> reconnect_timer_;
    
    // 统计信息
    mutable std::mutex stats_mutex_;
    uint32_t total_messages_sent_{0};
    uint32_t total_messages_received_{0};
    uint32_t total_connections_{0};
    uint32_t total_errors_{0};
};

} // namespace perception
