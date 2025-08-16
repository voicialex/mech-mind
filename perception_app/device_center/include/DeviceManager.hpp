#pragma once

#include "DeviceTypes.hpp"
#include "DeviceCommunicationAdapter.hpp"
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <mutex>

namespace device_center {

/**
 * @brief 统一的设备管理器基类
 * 
 * 设计原则：
 * - 提供DeviceServer和DeviceClient的共同功能
 * - 统一的设备生命周期管理
 * - 统一的事件处理机制
 * - 简化的配置管理
 */
class DeviceManager {
public:
    using EventHandlerPtr = DeviceEventHandler::Ptr;

    /**
     * @brief 管理器配置
     */
    struct Config {
        std::string service_id;
        std::string service_name;
        std::string local_address = "0.0.0.0";
        uint16_t local_port = 0;
        uint16_t discovery_port = 0;
        uint32_t heartbeat_interval = 5000; // ms
        uint32_t device_timeout = 30000; // ms
        bool enable_auto_cleanup = true;
        uint32_t cleanup_interval = 60000; // ms
        std::string log_level = "INFO";
        std::string log_file = "device_manager.log";
    };

    /**
     * @brief 设备上下文
     */
    struct DeviceContext {
        DeviceInfo info;
        DeviceStatusInfo status;
        std::string client_id;
        bool is_connected;
        uint64_t last_heartbeat;
        nlohmann::json config;
        
        DeviceContext() : is_connected(false), last_heartbeat(0) {}
    };

public:
    explicit DeviceManager(const Config& config);
    DeviceManager(); // 添加默认构造函数
    virtual ~DeviceManager();

    // 禁用拷贝
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager& operator=(const DeviceManager&) = delete;

    /**
     * @brief 初始化设备管理器
     * @param config_path 配置文件路径
     * @return 是否初始化成功
     */
    virtual bool Initialize(const std::string& config_path = "");

    /**
     * @brief 启动设备管理器
     * @return 是否启动成功
     */
    virtual bool Start();

    /**
     * @brief 停止设备管理器
     */
    virtual void Stop();

    /**
     * @brief 清理资源
     */
    virtual void Cleanup();

    /**
     * @brief 注册事件处理器
     * @param handler 事件处理器
     */
    void RegisterEventHandler(EventHandlerPtr handler);

    /**
     * @brief 注册设备
     * @param device_id 设备ID
     * @param device_info 设备信息
     * @param config 设备配置
     * @return 是否注册成功
     */
    virtual bool RegisterDevice(const std::string& device_id, 
                              const DeviceInfo& device_info = {},
                              const nlohmann::json& config = {});

    /**
     * @brief 注销设备
     * @param device_id 设备ID
     * @return 是否注销成功
     */
    virtual bool UnregisterDevice(const std::string& device_id);

    /**
     * @brief 更新设备状态
     * @param device_id 设备ID
     * @param status 设备状态
     * @return 是否更新成功
     */
    virtual bool UpdateDeviceStatus(const std::string& device_id, const DeviceStatusInfo& status);

    /**
     * @brief 获取设备信息
     * @param device_id 设备ID
     * @return 设备信息
     */
    virtual DeviceInfo GetDeviceInfo(const std::string& device_id) const;

    /**
     * @brief 获取设备状态
     * @param device_id 设备ID
     * @return 设备状态
     */
    virtual DeviceStatusInfo GetDeviceStatus(const std::string& device_id) const;

    /**
     * @brief 获取所有设备
     * @return 设备列表
     */
    virtual std::vector<DeviceInfo> GetAllDevices() const;

    /**
     * @brief 获取在线设备
     * @return 在线设备列表
     */
    virtual std::vector<DeviceInfo> GetOnlineDevices() const;

    /**
     * @brief 检查设备是否存在
     * @param device_id 设备ID
     * @return 是否存在
     */
    virtual bool HasDevice(const std::string& device_id) const;

    /**
     * @brief 检查设备是否在线
     * @param device_id 设备ID
     * @return 是否在线
     */
    virtual bool IsDeviceOnline(const std::string& device_id) const;

    /**
     * @brief 获取管理器配置
     * @return 配置
     */
    const Config& GetConfig() const { return config_; }

    /**
     * @brief 获取统计信息
     * @return 统计信息JSON
     */
    virtual nlohmann::json GetStatistics() const;

    /**
     * @brief 检查是否已初始化
     * @return 是否已初始化
     */
    bool IsInitialized() const { return initialized_; }

    /**
     * @brief 检查是否正在运行
     * @return 是否正在运行
     */
    bool IsRunning() const { return running_; }

protected:
    // 受保护的方法，供子类使用
    std::shared_ptr<DeviceCommunicationAdapter> GetCommunicationAdapter() const { return comm_adapter_; }
    
    /**
     * @brief 处理设备消息（子类实现）
     * @param message 设备消息
     */
    virtual void HandleDeviceMessage(const DeviceCommunicationAdapter::DeviceMessage& message) = 0;
    
    /**
     * @brief 处理连接状态变化（子类实现）
     * @param service_id 服务ID
     * @param state 连接状态
     */
    virtual void HandleConnectionStateChanged(const std::string& service_id, 
                                           DeviceCommunicationAdapter::ConnectionState state) = 0;
    
    /**
     * @brief 处理通信错误（子类实现）
     * @param service_id 服务ID
     * @param error_code 错误码
     */
    virtual void HandleCommunicationError(const std::string& service_id, uint16_t error_code) = 0;

    /**
     * @brief 通知事件处理器
     */
    void NotifyDeviceConnectionChanged(const std::string& device_id, bool connected);
    void NotifyDeviceStatusChanged(const std::string& device_id, DeviceStatus old_status, DeviceStatus new_status);
    void NotifyDeviceDataReceived(const DeviceData& device_data);
    void NotifyDeviceError(const std::string& device_id, uint16_t error_code, const std::string& error_message);

    // 受保护的成员变量，供子类访问
    Config config_;
    std::shared_ptr<DeviceCommunicationAdapter> comm_adapter_;
    
    // 设备管理
    std::unordered_map<std::string, DeviceContext> devices_;
    mutable std::mutex devices_mutex_;

    // 事件处理器
    EventHandlerPtr event_handler_;
    mutable std::mutex event_handler_mutex_;

    // 状态管理
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    std::atomic<bool> heartbeat_running_{false};
    
    // 线程
    std::thread heartbeat_thread_;
    
    // 统计信息
    uint64_t start_time_;
    std::atomic<uint32_t> total_devices_registered_{0};
    std::atomic<uint32_t> total_messages_{0};
    std::atomic<uint32_t> total_errors_{0};

private:
    // 私有方法
    void LoadConfig(const std::string& config_path);
    void InitializeCommunication();
    void StartHeartbeatMonitor();
    void StopHeartbeatMonitor();
    void ProcessHeartbeat();
    void CleanupOfflineDevices();
    
    // 通信回调处理
    void OnMessageReceived(const DeviceCommunicationAdapter::DeviceMessage& message);
    void OnConnectionStateChanged(const std::string& service_id, DeviceCommunicationAdapter::ConnectionState state);
    void OnCommunicationError(const std::string& service_id, uint16_t error_code);
    
    // 工具方法
    bool ValidateDeviceRegistration(const std::string& device_id);
    std::string GenerateDeviceId(const std::string& device_name);
    uint64_t GetCurrentTimestamp();
};

} // namespace device_center
