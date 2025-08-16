#pragma once

#include "DeviceManager.hpp"
#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace device_center {

/**
 * @brief 设备服务器 - 管理设备注册、发现和控制
 * 
 * 优化后的功能特性：
 * - 继承自DeviceManager，复用通用功能
 * - 专注于服务器特有的业务逻辑
 * - 简化的接口设计
 * - 统一的事件处理机制
 */
class DeviceServer : public DeviceManager {
public:
    /**
     * @brief 服务器配置
     */
    struct ServerConfig : public Config {
        uint32_t max_devices = 100;
        uint32_t max_clients = 50;
        bool enable_device_discovery = true;
        bool enable_auto_approval = true;
    };

public:
    explicit DeviceServer(const ServerConfig& config);
    DeviceServer(); // 默认构造函数
    ~DeviceServer() override;

    /**
     * @brief 初始化设备服务器
     * @param config_path 配置文件路径
     * @return 是否初始化成功
     */
    bool Initialize(const std::string& config_path = "") override;

    /**
     * @brief 注册设备（服务器版本）
     * @param request 设备注册请求
     * @return 设备注册响应
     */
    DeviceRegisterResponse RegisterDevice(const DeviceRegisterRequest& request);

    /**
     * @brief 发现设备
     * @param request 设备发现请求
     * @return 设备发现响应
     */
    DeviceDiscoveryResponse DiscoverDevices(const DeviceDiscoveryRequest& request);

    /**
     * @brief 控制设备
     * @param request 设备控制请求
     * @return 设备控制响应
     */
    DeviceControlResponse ControlDevice(const DeviceControlRequest& request);

    /**
     * @brief 发送设备命令
     * @param command 设备命令
     * @return 设备响应
     */
    DeviceResponse SendDeviceCommand(const DeviceCommand& command);

    /**
     * @brief 接收设备数据
     * @param device_data 设备数据
     */
    void ReceiveDeviceData(const DeviceData& device_data);

    /**
     * @brief 广播设备数据
     * @param device_data 设备数据
     * @param target_clients 目标客户端列表（为空则广播给所有客户端）
     */
    void BroadcastDeviceData(const DeviceData& device_data, 
                           const std::vector<std::string>& target_clients = {});

    /**
     * @brief 获取服务器统计信息
     * @return 统计信息JSON
     */
    nlohmann::json GetServerStatistics() const;

    /**
     * @brief 获取服务器配置
     * @return 服务器配置
     */
    const ServerConfig& GetServerConfig() const { return server_config_; }

protected:
    // 实现DeviceManager的虚函数
    void HandleDeviceMessage(const DeviceCommunicationAdapter::DeviceMessage& message) override;
    void HandleConnectionStateChanged(const std::string& service_id, 
                                   DeviceCommunicationAdapter::ConnectionState state) override;
    void HandleCommunicationError(const std::string& service_id, uint16_t error_code) override;

private:
    // 服务器特有的私有方法
    void HandleDeviceRegistration(const DeviceRegisterRequest& request);
    void HandleDeviceDiscovery(const DeviceDiscoveryRequest& request);
    void HandleDeviceControl(const DeviceControlRequest& request);
    void HandleDeviceCommand(const DeviceCommand& command);
    void HandleDeviceStatusUpdate(const std::string& device_id, const DeviceStatusInfo& status);
    void HandleDeviceData(const DeviceData& device_data);
    
    // 工具方法
    bool ValidateDeviceRegistration(const DeviceRegisterRequest& request);
    std::string GenerateDeviceId(const DeviceRegisterRequest& request);
    void UpdateDeviceConnectionStatus(const std::string& device_id, bool connected);

private:
    // 服务器特有的成员变量
    ServerConfig server_config_;
    
    // 客户端管理
    std::unordered_map<std::string, std::vector<std::string>> client_devices_; // client_id -> device_ids
    mutable std::mutex client_devices_mutex_;
    
    // 服务器统计
    std::atomic<uint32_t> total_connections_{0};
    std::atomic<uint32_t> total_commands_processed_{0};
};

} // namespace device_center
