#pragma once

#include "DeviceManager.hpp"
#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace device_center {

/**
 * @brief 设备客户端 - 连接设备服务器并管理本地设备
 * 
 * 优化后的功能特性：
 * - 继承自DeviceManager，复用通用功能
 * - 专注于客户端特有的业务逻辑
 * - 简化的接口设计
 * - 统一的事件处理机制
 */
class DeviceClient : public DeviceManager {
public:
    /**
     * @brief 客户端配置
     */
    struct ClientConfig : public Config {
        std::string server_address = "127.0.0.1";
        uint16_t server_port = 9090;
        uint32_t reconnect_interval = 5000; // ms
        uint32_t max_reconnect_attempts = 10;
        bool enable_auto_reconnect = true;
        bool enable_auto_discovery = true;
    };

    /**
     * @brief 设备配置
     */
    struct DeviceConfig {
        std::string device_id;
        std::string device_name;
        std::string device_model;
        std::string device_version;
        std::vector<DeviceCapability> capabilities;
        nlohmann::json config;
        bool auto_register = true;
        bool auto_heartbeat = true;
    };

public:
    explicit DeviceClient(const ClientConfig& config);
    DeviceClient(); // 默认构造函数
    ~DeviceClient() override;

    /**
     * @brief 初始化设备客户端
     * @param config_path 配置文件路径
     * @return 是否初始化成功
     */
    bool Initialize(const std::string& config_path = "") override;

    /**
     * @brief 连接到设备服务器
     * @param server_id 服务器ID（可选，为空则自动发现）
     * @return 是否连接成功
     */
    bool ConnectToServer(const std::string& server_id = "");

    /**
     * @brief 断开与服务器的连接
     */
    void DisconnectFromServer();

    /**
     * @brief 注册设备（客户端版本）
     * @param device_id 设备ID
     * @param config 设备配置
     * @return 是否注册成功
     */
    bool RegisterDevice(const std::string& device_id, const nlohmann::json& config = {});

    /**
     * @brief 注册设备（使用设备配置）
     * @param device_config 设备配置
     * @return 是否注册成功
     */
    bool RegisterDevice(const DeviceConfig& device_config);

    /**
     * @brief 上报设备状态
     * @param device_id 设备ID
     * @param status 设备状态
     * @return 是否上报成功
     */
    bool ReportDeviceStatus(const std::string& device_id, const DeviceStatusInfo& status);

    /**
     * @brief 上报设备数据
     * @param device_data 设备数据
     * @return 是否上报成功
     */
    bool ReportDeviceData(const DeviceData& device_data);

    /**
     * @brief 响应设备命令
     * @param device_id 设备ID
     * @param command_id 命令ID
     * @param response 设备响应
     * @return 是否响应成功
     */
    bool RespondToCommand(const std::string& device_id, 
                         const std::string& command_id, 
                         const DeviceResponse& response);

    /**
     * @brief 发现服务器
     * @param server_name 服务器名称（可选）
     * @return 服务器列表
     */
    std::vector<std::string> DiscoverServers(const std::string& server_name = "");

    /**
     * @brief 检查是否连接到服务器
     * @return 是否已连接
     */
    bool IsConnectedToServer() const;

    /**
     * @brief 获取连接状态
     * @return 连接状态信息
     */
    nlohmann::json GetConnectionStatus() const;

    /**
     * @brief 获取客户端统计信息
     * @return 统计信息JSON
     */
    nlohmann::json GetClientStatistics() const;

    /**
     * @brief 获取客户端配置
     * @return 客户端配置
     */
    const ClientConfig& GetClientConfig() const { return client_config_; }

    /**
     * @brief 设置设备配置
     * @param device_id 设备ID
     * @param config 设备配置
     * @return 是否设置成功
     */
    bool SetDeviceConfig(const std::string& device_id, const nlohmann::json& config);

    /**
     * @brief 获取设备配置
     * @param device_id 设备ID
     * @return 设备配置
     */
    nlohmann::json GetDeviceConfig(const std::string& device_id) const;

    /**
     * @brief 启用设备心跳
     * @param device_id 设备ID
     * @param interval_ms 心跳间隔（毫秒）
     * @return 是否启用成功
     */
    bool EnableDeviceHeartbeat(const std::string& device_id, uint32_t interval_ms = 5000);

    /**
     * @brief 禁用设备心跳
     * @param device_id 设备ID
     * @return 是否禁用成功
     */
    bool DisableDeviceHeartbeat(const std::string& device_id);

protected:
    // 实现DeviceManager的虚函数
    void HandleDeviceMessage(const DeviceCommunicationAdapter::DeviceMessage& message) override;
    void HandleConnectionStateChanged(const std::string& service_id, 
                                   DeviceCommunicationAdapter::ConnectionState state) override;
    void HandleCommunicationError(const std::string& service_id, uint16_t error_code) override;

private:
    // 客户端特有的私有方法
    void HandleServerCommand(const DeviceCommand& command);
    void HandleServerStatusRequest(const std::string& device_id);
    void HandleServerDataRequest(const std::string& device_id);
    void AutoReconnect();
    void SendHeartbeat();
    
    // 工具方法
    bool ValidateDeviceRegistration(const std::string& device_id);
    void UpdateConnectionStatus(bool connected, const std::string& server_id = "");

private:
    // 客户端特有的成员变量
    ClientConfig client_config_;
    
    // 连接状态
    std::atomic<bool> connected_{false};
    std::string connected_server_id_;
    
    // 设备配置管理
    std::unordered_map<std::string, DeviceConfig> device_configs_;
    std::unordered_map<std::string, bool> device_heartbeat_enabled_;
    mutable std::mutex device_configs_mutex_;
    
    // 重连管理
    std::atomic<uint32_t> reconnect_attempts_{0};
    std::atomic<uint64_t> last_reconnect_time_{0};
    
    // 客户端统计
    std::atomic<uint32_t> total_status_reports_{0};
    std::atomic<uint32_t> total_data_reports_{0};
    std::atomic<uint32_t> total_commands_processed_{0};
};

} // namespace device_center
