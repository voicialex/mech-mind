#pragma once

#include "communication/interfaces/IEndpointService.hpp"
#include "communication/interfaces/ITransport.hpp"
#include "message/IMessageProtocol.hpp"
#include <memory>
#include <atomic>
#include <mutex>
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
    explicit EndpointService(const EndpointConfig& config);

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
    const EndpointConfig& GetConfig() const override;

    /**
     * @brief 获取服务配置（兼容性方法）
     * @return 服务配置
     */
    const EndpointConfig& GetServiceConfig() const { return config_; }

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
    
    // 消息处理
    bool ProcessIncomingMessage(const std::string& endpoint_id, const std::vector<uint8_t>& data);
    bool ProcessOutgoingMessage(const std::string& target_id, const std::vector<uint8_t>& data);
    
    // 心跳支持
    void EnableHeartbeat(bool enable) { enable_heartbeat_ = enable; }
    bool IsHeartbeatEnabled() const { return enable_heartbeat_; }
    bool IsHeartbeatMessage(const std::vector<uint8_t>& message_data) const;
    
    // 消息路由器访问
    std::shared_ptr<IMessageRouter> GetMessageRouter() const { return message_router_; }
    
    // 统计信息访问
    EndpointStatistics& GetStatistics() { return statistics_; }

private:
    /**
     * @brief 私有方法
     */
    void InitializeMessageProtocol();
    void InitializeMessageRouter();
    
    void OnMessageReceived(const std::string& service_id, const std::vector<uint8_t>& data);
    void OnConnectionChanged(const std::string& service_id, bool connected, const ConnectionInfo& connection_info);
    void OnError(const std::string& service_id, uint16_t error_code, const std::string& error_message);

protected:
    /**
     * @brief 配置和状态
     */
    EndpointConfig config_;
    std::atomic<EndpointState> state_{EndpointState::Stopped};
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    
    // 底层服务
    std::shared_ptr<ITransport> transport_;
    std::shared_ptr<IMessageRouter> message_router_;
    
    // 统一的事件处理器
    EventHandler::Ptr event_handler_;
    
    // 内部事件处理器 - 直接传递事件
    class InternalEventHandler : public ITransport::EventHandler {
    public:
        explicit InternalEventHandler(EndpointService* service) : service_(service) {}
        
        void OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) override {
            if (service_) {
                service_->OnMessageReceived(endpoint_id, message_data);
            }
        }
        
        void OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) override {
            if (service_) {
                service_->OnConnectionChanged(endpoint_id, connected, connection_info);
            }
        }
        
        void OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) override {
            if (service_) {
                service_->OnError(endpoint_id, error_code, error_message);
            }
        }
        
    private:
        EndpointService* service_;
    };
    
    std::shared_ptr<InternalEventHandler> internal_event_handler_;
    
    // 状态管理
    std::unordered_map<std::string, bool> endpoint_connections_;
    mutable std::mutex connections_mutex_;
    
    // 统一的统计信息
    EndpointStatistics statistics_;
    
    // 心跳相关（可选）
    bool enable_heartbeat_ = false;
};

} // namespace perception
