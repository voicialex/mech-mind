#pragma once

#include "ConnectionTypes.hpp"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace perception {

/**
 * @brief 传输层抽象接口
 * 
 * 设计原则：
 * - 纯虚接口，不包含实现细节
 * - 支持多种传输协议（TCP、UDP、WebSocket等）
 * - 异步操作，支持回调机制
 * - 连接管理和状态监控
 */
class ITransport {
public:
    using Ptr = std::shared_ptr<ITransport>;
    
    /**
     * @brief 统一事件处理器接口
     */
    class EventHandler {
    public:
        using Ptr = std::shared_ptr<EventHandler>;
        
        virtual ~EventHandler() = default;
        
        /**
         * @brief 消息接收事件
         * @param endpoint_id 端点ID
         * @param message_data 消息数据
         */
        virtual void OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) = 0;
        
        /**
         * @brief 连接状态变化事件
         * @param endpoint_id 端点ID
         * @param connected 是否连接
         * @param connection_info 连接信息
         */
        virtual void OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) = 0;
        
        /**
         * @brief 错误事件
         * @param endpoint_id 端点ID
         * @param error_code 错误码
         * @param error_message 错误消息
         */
        virtual void OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) = 0;
    };
    
    virtual ~ITransport() = default;
    
    /**
     * @brief 初始化传输层
     * @return 是否初始化成功
     */
    virtual bool Initialize() = 0;
    
    /**
     * @brief 启动传输服务
     * @return 是否启动成功
     */
    virtual bool Start() = 0;
    
    /**
     * @brief 停止传输服务
     */
    virtual void Stop() = 0;
    
    /**
     * @brief 连接到远程服务
     * @param service_id 服务ID
     * @param address 地址
     * @param port 端口
     * @return 是否连接成功
     */
    virtual bool Connect(const std::string& service_id, const std::string& address, uint16_t port) = 0;
    
    /**
     * @brief 断开连接
     * @param service_id 服务ID
     */
    virtual void Disconnect(const std::string& service_id) = 0;
    
    /**
     * @brief 发送消息
     * @param target_id 目标ID
     * @param data 消息数据
     * @return 是否发送成功
     */
    virtual bool SendMessage(const std::string& target_id, const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief 广播消息
     * @param data 消息数据
     * @param target_filter 目标过滤器（可选）
     * @return 是否广播成功
     */
    virtual bool BroadcastMessage(const std::vector<uint8_t>& data, const std::string& target_filter = "") = 0;
    
    /**
     * @brief 注册事件处理器
     * @param handler 事件处理器
     */
    virtual void RegisterEventHandler(EventHandler::Ptr handler) = 0;
    
    /**
     * @brief 获取连接信息
     * @param service_id 服务ID
     * @return 连接信息
     */
    virtual ConnectionInfo GetConnectionInfo(const std::string& service_id) const = 0;
    
    /**
     * @brief 获取所有连接信息
     * @return 连接信息列表
     */
    virtual std::vector<ConnectionInfo> GetAllConnections() const = 0;
    
    /**
     * @brief 检查连接状态
     * @param service_id 服务ID
     * @return 是否已连接
     */
    virtual bool IsConnected(const std::string& service_id) const = 0;
    
    /**
     * @brief 检查是否正在运行
     * @return 是否正在运行
     */
    virtual bool IsRunning() const = 0;
};

/**
 * @brief 服务发现接口
 */
class IServiceDiscovery {
public:
    using Ptr = std::shared_ptr<IServiceDiscovery>;
    
    using ServiceDiscoveredCallback = std::function<void(const EndpointIdentity&)>;
    
    virtual ~IServiceDiscovery() = default;

    /**
     * @brief 停止服务发现（停止接收与广播线程）
     */
    virtual void Stop() = 0;
    
    /**
     * @brief 注册本地服务
     * @param service_info 服务信息
     * @return 是否注册成功
     */
    virtual bool RegisterService(const EndpointIdentity& service_info) = 0;
    
    /**
     * @brief 注销本地服务
     */
    virtual void UnregisterService() = 0;
    
    /**
     * @brief 发现服务
     * @param service_name 服务名称（可选）
     * @param callback 新服务发现回调（可选，传入后内部确保接收线程运行）
     * @return 发现的服务列表
     */
    virtual std::vector<EndpointIdentity> DiscoverServices(const std::string& service_name = "", ServiceDiscoveredCallback callback = nullptr) = 0;
    
    /**
     * @brief 检查是否正在运行
     * @return 是否正在运行
     */
    virtual bool IsRunning() const = 0;
};

} // namespace perception
