#pragma once

#include "communication/interfaces/IEndpointService.hpp"
#include "communication/interfaces/ITransport.hpp"
#include "message/IMessageProtocol.hpp"
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <string>

namespace perception {

/**
 * @brief 统一的端点服务统计信息结构体
 */
struct EndpointStatistics {
    // 基础统计信息
    std::atomic<uint32_t> messages_sent{0};
    std::atomic<uint32_t> messages_received{0};
    std::atomic<uint32_t> connections_established{0};
    std::atomic<uint32_t> errors{0};
    
    // 时间统计
    uint64_t start_time{0};
    
    // 客户端特有统计
    std::atomic<uint32_t> total_connections{0};
    std::atomic<uint32_t> total_disconnections{0};
    std::atomic<uint32_t> reconnect_attempts{0};
    std::atomic<uint64_t> last_reconnect_time{0};
    
    // 服务器特有统计
    std::atomic<uint32_t> total_clients_registered{0};
    std::atomic<uint32_t> total_clients_connected{0};
    std::atomic<uint32_t> total_broadcasts{0};
    
    // 重置统计信息
    void Reset() {
        messages_sent = 0;
        messages_received = 0;
        connections_established = 0;
        errors = 0;
        total_connections = 0;
        total_disconnections = 0;
        reconnect_attempts = 0;
        last_reconnect_time = 0;
        total_clients_registered = 0;
        total_clients_connected = 0;
        total_broadcasts = 0;
        start_time = 0;
    }
    
    // 获取运行时间
    uint64_t GetUptime() const {
        if (start_time == 0) return 0;
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() - start_time;
    }
};

/**
 * @brief 端点服务基类 - 专注于TCP连接和通信功能
 * 
 * 设计原则：
 * - 只负责TCP连接管理和消息传输
 * - 不包含服务发现功能
 * - 提供统一的连接和消息接口
 * - 支持客户端和服务器模式
 * - 统一的事件处理器管理
 */
class EndpointService : public IEndpointService {
public:
    // 允许检查器读取受保护成员以生成统计信息，而无需在接口上暴露方法
    friend class ServiceInspector;
    /**
     * @brief 构造函数
     * @param config 端点配置
     */
    explicit EndpointService(const EndpointIdentity& config);

    /**
     * @brief 析构函数
     */
    ~EndpointService() override;

    /**
     * @brief 初始化端点服务
     * @return 是否初始化成功
     */
    bool Initialize() override;

    /**
     * @brief 启动端点服务
     * @return 是否启动成功
     */
    bool Start() override;

    /**
     * @brief 停止端点服务
     */
    void Stop() override;

    /**
     * @brief 清理资源
     */
    void Cleanup() override;

    /**
     * @brief 发送消息
     * @param target_id 目标ID
     * @param message_data 消息数据
     * @param timeout_ms 超时时间（毫秒），0表示不等待响应
     * @return 是否发送成功
     */
    bool SendMessage(const std::string& target_id, const std::vector<uint8_t>& message_data, uint32_t timeout_ms = 0) override;

    /**
     * @brief 广播消息
     * @param message_data 消息数据
     * @param target_name 目标名称（可选）
     */
    void BroadcastMessage(const std::vector<uint8_t>& message_data, const std::string& target_name = "") override;

    /**
     * @brief 注册事件处理器
     * @param handler 事件处理器
     */
    void RegisterEventHandler(EventHandler::Ptr handler) override;

    /**
     * @brief 检查端点是否在线
     * @param endpoint_id 端点ID
     * @return 是否在线
     */
    bool IsEndpointOnline(const std::string& endpoint_id) const override;

    /**
     * @brief 获取端点状态
     * @return 端点状态
     */
    EndpointState GetState() const override;

    /**
     * @brief 获取配置
     * @return 配置
     */
    const EndpointIdentity& GetConfig() const override;

    /**
     * @brief 获取服务配置（兼容性方法）
     * @return 服务配置
     */
    const EndpointIdentity& GetServiceConfig() const { return config_; }

    /**
     * @brief 检查是否正在运行
     * @return 是否正在运行
     */
    bool IsRunning() const override;

protected:
    /**
     * @brief 初始化传输层
     * @return 是否初始化成功
     */
    bool InitializeTransport();

    /**
     * @brief 启动传输层
     * @return 是否启动成功
     */
    bool StartTransport();

    /**
     * @brief 停止传输层
     */
    void StopTransport();

    /**
     * @brief 清理传输层
     */
    void CleanupTransport();

    /**
     * @brief 子类可以访问的受保护方法
     * @param state 端点状态
     */
    void SetState(EndpointState state);
    
    // 工具方法
    std::string GenerateEndpointId() const;
    uint64_t GetCurrentTimestamp() const;
    
    // 底层服务访问
    std::shared_ptr<ITransport> GetTransport() const { return transport_; }
    
    // 配置和状态访问
    bool IsInitialized() const { return initialized_.load(); }
    bool IsServiceRunning() const { return running_.load(); }
    
    // 连接管理
    bool IsConnected(const std::string& endpoint_id) const;
    std::vector<std::string> GetConnectedEndpoints() const;
    

    
    // 心跳支持（子类实现）
    virtual void EnableHeartbeat(bool enable) = 0;
    virtual bool IsHeartbeatEnabled() const = 0;
    
    // 消息路由器支持
    void RegisterHeartbeatCallbacks();
    void UnregisterHeartbeatCallbacks();
    std::shared_ptr<MessageRouter> GetMessageRouter() const { return message_router_; }
    
    // 心跳处理纯虚函数（子类实现具体逻辑）
    virtual void OnHeartbeatRequest(std::shared_ptr<ITransport> transport, const std::string& endpoint_id, 
                                   uint16_t message_id, uint8_t sub_message_id, const std::vector<uint8_t>& payload) = 0;
    virtual void OnHeartbeatResponse(std::shared_ptr<ITransport> transport, const std::string& endpoint_id, 
                                    uint16_t message_id, uint8_t sub_message_id, const std::vector<uint8_t>& payload) = 0;
    
    
    // 统计信息访问
    EndpointStatistics& GetStatistics() { return statistics_; }

protected:
    /**
     * @brief 配置和状态
     */
    EndpointIdentity config_;
    std::atomic<EndpointState> state_{EndpointState::Stopped};
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    
    // 底层服务
    std::shared_ptr<ITransport> transport_;
    
    // 统一的事件处理器
    EventHandler::Ptr event_handler_;
    
    // 内部事件处理器声明
    class InternalEventHandler;
    std::shared_ptr<ITransport::EventHandler> internal_event_handler_;
    
    // 状态管理
    std::unordered_map<std::string, bool> endpoint_connections_;
    mutable std::mutex connections_mutex_;
    
    // 统一的统计信息
    EndpointStatistics statistics_;
    
    // 消息路由器
    std::shared_ptr<MessageRouter> message_router_;
    
    // 当前正在处理的端点ID（用于回调函数获取发送者ID）
    std::string current_processing_endpoint_id_;
    
public:
    // 客户端心跳信息结构体
    struct ClientHeartbeatInfo {
        uint64_t last_request_time{0};      // 最后发送心跳请求的时间
        uint64_t last_response_time{0};     // 最后收到心跳响应的时间
        uint32_t consecutive_missed{0};     // 连续未响应次数
        uint32_t total_requests{0};         // 总请求数
        uint32_t total_responses{0};        // 总响应数
        bool is_alive{true};               // 是否存活
        
        // 获取响应率
        double GetResponseRate() const {
            return total_requests > 0 ? static_cast<double>(total_responses) / total_requests : 0.0;
        }
        
        // 获取最后活动时间
        uint64_t GetLastActivity() const {
            return std::max(last_request_time, last_response_time);
        }
        
        // 比较操作符（用于排序）
        bool operator<(const ClientHeartbeatInfo& other) const {
            return GetLastActivity() < other.GetLastActivity();
        }
    };
    

};

} // namespace perception
