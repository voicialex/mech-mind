#pragma once

#include "communication/endpoints/clients/EndpointClient.hpp"
#include "communication/discovery/UdpServiceDiscovery.hpp"
#include "communication/interfaces/ConnectionTypes.hpp"
#include <memory>
#include <string>
#include <functional>

namespace perception {

/**
 * @brief Client节点 - 包含服务发现和客户端端点
 * 
 * 设计原则：
 * - 作为客户端节点
 * - 监听服务发现广播
 * - 管理多个客户端连接
 * - 提供统一的客户端接口
 */
class ClientNode {
public:
    using Ptr = std::shared_ptr<ClientNode>;
    
    /**
     * @brief Client节点配置
     */
    struct Config {
        // 服务发现配置
        std::string discovery_address = "0.0.0.0";
        uint16_t discovery_port = 8080;
        uint32_t discovery_timeout = 10000; // 毫秒
        
        // 设备客户端（原 center_device_client）
        std::string device_client_id = "device_client";
        std::string device_client_name = "Device Client";
        
        // 控制器客户端
        std::string controller_client_id = "controller_client";
        std::string controller_client_name = "Controller Client";
        bool enable_controller_client = false; // 是否启用控制器客户端
        
        Config() = default;
    };

public:
    explicit ClientNode(const Config& config);
    ~ClientNode();

    /**
     * @brief 初始化Client节点
     * @return 是否初始化成功
     */
    bool Initialize();
    
    /**
     * @brief 启动Client节点
     * @return 是否启动成功
     */
    bool Start();
    
    /**
     * @brief 停止Client节点
     */
    void Stop();

    /**
     * @brief 获取设备客户端实例
     * @return 设备客户端指针
     */
    std::shared_ptr<EndpointClient> GetDeviceClient() const { return device_client_; }

    /**
     * @brief 获取控制器客户端实例
     * @return 控制器客户端指针
     */
    std::shared_ptr<EndpointClient> GetControllerClient() const { return controller_client_; }

private:
    // 内部事件处理器（设备客户端）
    class DeviceClientEventHandler : public ITransport::EventHandler {
    public:
        explicit DeviceClientEventHandler(ClientNode* node) : node_(node) {}
        void OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) override;
        void OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) override;
        void OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) override;
    private:
        ClientNode* node_;
    };

    // 内部事件处理器（控制器客户端）
    class ControllerClientEventHandler : public ITransport::EventHandler {
    public:
        explicit ControllerClientEventHandler(ClientNode* node) : node_(node) {}
        void OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) override;
        void OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) override;
        void OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) override;
    private:
        ClientNode* node_;
    };

private:
    // 服务发现回调
    void OnServiceDiscovered(const EndpointIdentity& service_info);

private:
    Config config_;
    
    // 服务发现组件
    std::shared_ptr<UdpServiceDiscovery> service_discovery_;
    
    // 客户端端点组件
    std::shared_ptr<EndpointClient> device_client_;
    std::shared_ptr<EndpointClient> controller_client_;
    
    // 事件处理器
    std::shared_ptr<DeviceClientEventHandler> device_event_handler_;
    std::shared_ptr<ControllerClientEventHandler> controller_event_handler_;
    
    // 状态管理
    bool initialized_ = false;
    bool running_ = false;
};

} // namespace perception
