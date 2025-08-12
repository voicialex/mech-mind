#pragma once

#include "DeviceTypes.hpp"
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <mutex>

namespace device_center {

/**
 * @brief 设备客户端 - 连接设备服务器并管理本地设备
 * 
 * 功能特性：
 * - 连接设备服务器
 * - 设备注册和注销
 * - 设备状态上报
 * - 设备控制响应
 * - 设备数据上传
 * - 自动重连机制
 */
class DeviceClient {
public:
    using DeviceHandlerPtr = DeviceHandler::Ptr;
    using EventHandlerPtr = DeviceEventHandler::Ptr;
    using CommandCallback = std::function<DeviceResponse(const DeviceCommand&)>;
    using StatusCallback = std::function<void(const DeviceStatusInfo&)>;

    /**
     * @brief 客户端配置
     */
    struct Config {
        std::string service_id = "device_client_001";
        std::string service_name = "Device Client";
        std::string server_address = "127.0.0.1";
        uint16_t server_port = 9090;
        uint16_t local_port = 9092;
        uint32_t reconnect_interval = 5000; // ms
        uint32_t heartbeat_interval = 5000; // ms
        uint32_t max_reconnect_attempts = 10;
        bool enable_auto_reconnect = true;
        std::string log_level = "INFO";
        std::string log_file = "device_client.log";
    };

    /**
     * @brief 设备配置
     */
    struct DeviceConfig {
        std::string device_id;
        std::string device_name;
        DeviceType device_type;
        std::string device_model;
        std::string device_version;
        std::vector<DeviceCapability> capabilities;
        nlohmann::json config;
        bool auto_register = true;
        bool auto_heartbeat = true;
    };

public:
    explicit DeviceClient(const Config& config);
    DeviceClient(); // 默认构造函数
    ~DeviceClient();

    // 禁用拷贝
    DeviceClient(const DeviceClient&) = delete;
    DeviceClient& operator=(const DeviceClient&) = delete;

    /**
     * @brief 初始化设备客户端
     * @param config_path 配置文件路径
     * @return 是否初始化成功
     */
    bool Initialize(const std::string& config_path = "");

    /**
     * @brief 连接到设备服务器
     * @param server_id 服务器ID
     * @return 是否连接成功
     */
    bool ConnectToServer(const std::string& server_id = "");

    /**
     * @brief 断开与服务器的连接
     */
    void DisconnectFromServer();

    /**
     * @brief 启动客户端
     * @return 是否启动成功
     */
    bool Start();

    /**
     * @brief 停止客户端
     */
    void Stop();

    /**
     * @brief 运行客户端循环
     */
    void Run();

    /**
     * @brief 清理资源
     */
    void Cleanup();

    /**
     * @brief 注册设备
     * @param device_id 设备ID
     * @param device_type 设备类型
     * @param config 设备配置
     * @return 是否注册成功
     */
    bool RegisterDevice(const std::string& device_id, 
                       DeviceType device_type, 
                       const nlohmann::json& config = {});

    /**
     * @brief 注册设备（使用设备配置）
     * @param device_config 设备配置
     * @return 是否注册成功
     */
    bool RegisterDevice(const DeviceConfig& device_config);

    /**
     * @brief 注销设备
     * @param device_id 设备ID
     * @return 是否注销成功
     */
    bool UnregisterDevice(const std::string& device_id);

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
     * @brief 注册设备处理器
     * @param device_id 设备ID
     * @param handler 设备处理器
     * @return 是否注册成功
     */
    bool RegisterDeviceHandler(const std::string& device_id, DeviceHandlerPtr handler);

    /**
     * @brief 注册事件处理器
     * @param handler 事件处理器
     */
    void RegisterEventHandler(EventHandlerPtr handler);

    /**
     * @brief 注册命令回调
     * @param device_id 设备ID
     * @param callback 命令回调函数
     * @return 是否注册成功
     */
    bool RegisterCommandCallback(const std::string& device_id, CommandCallback callback);

    /**
     * @brief 注册状态回调
     * @param device_id 设备ID
     * @param callback 状态回调函数
     * @return 是否注册成功
     */
    bool RegisterStatusCallback(const std::string& device_id, StatusCallback callback);

    /**
     * @brief 发现服务器
     * @param server_name 服务器名称（可选）
     * @return 服务器列表
     */
    std::vector<std::string> DiscoverServers(const std::string& server_name = "");

    /**
     * @brief 获取已注册的设备
     * @return 设备列表
     */
    std::vector<DeviceInfo> GetRegisteredDevices() const;

    /**
     * @brief 获取设备信息
     * @param device_id 设备ID
     * @return 设备信息
     */
    DeviceInfo GetDeviceInfo(const std::string& device_id) const;

    /**
     * @brief 获取设备状态
     * @param device_id 设备ID
     * @return 设备状态
     */
    DeviceStatusInfo GetDeviceStatus(const std::string& device_id) const;

    /**
     * @brief 检查设备是否已注册
     * @param device_id 设备ID
     * @return 是否已注册
     */
    bool IsDeviceRegistered(const std::string& device_id) const;

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
    Config GetConfig() const;

    /**
     * @brief 检查客户端是否运行
     * @return 是否运行
     */
    bool IsRunning() const;

    /**
     * @brief 检查客户端是否已初始化
     * @return 是否已初始化
     */
    bool IsInitialized() const;

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

private:
    // 私有方法
    void LoadConfig(const std::string& config_path);
    void InitializeCommunication();
    void StartHeartbeat();
    void StopHeartbeat();
    void ProcessHeartbeat();
    void HandleServerMessage(const std::vector<uint8_t>& message_data);
    void HandleDeviceRegistration(const DeviceRegisterRequest& request);
    void HandleDeviceDiscovery(const DeviceDiscoveryRequest& request);
    void HandleDeviceControl(const DeviceControlRequest& request);
    void HandleDeviceCommand(const DeviceCommand& command);
    void HandleDeviceStatusUpdate(const std::string& device_id, const DeviceStatusInfo& status);
    void HandleDeviceData(const DeviceData& device_data);
    void NotifyDeviceStatusChanged(const std::string& device_id, 
                                 DeviceStatus old_status, 
                                 DeviceStatus new_status);
    void NotifyDeviceDataReceived(const DeviceData& device_data);
    void NotifyDeviceError(const std::string& device_id, 
                          uint16_t error_code, 
                          const std::string& error_message);
    DeviceHandlerPtr GetDeviceHandler(const std::string& device_id);
    bool ValidateDeviceRegistration(const std::string& device_id, DeviceType device_type);
    void AutoReconnect();
    void SendHeartbeat();

private:
    Config config_;
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    std::atomic<bool> connected_{false};
    
    // 设备管理
    std::unordered_map<std::string, DeviceInfo> registered_devices_;
    std::unordered_map<std::string, DeviceStatusInfo> device_status_;
    std::unordered_map<std::string, DeviceConfig> device_configs_;
    std::unordered_map<std::string, bool> device_heartbeat_enabled_;
    mutable std::mutex devices_mutex_;
    
    // 处理器管理
    std::unordered_map<std::string, DeviceHandlerPtr> device_handlers_;
    std::unordered_map<std::string, CommandCallback> command_callbacks_;
    std::unordered_map<std::string, StatusCallback> status_callbacks_;
    EventHandlerPtr event_handler_;
    mutable std::mutex handlers_mutex_;
    
    // 通信管理
    struct CommunicationDeleter {
        void operator()(void* ptr) const;
    };
    std::unique_ptr<void, CommunicationDeleter> comm_interface_; // 通信接口（避免直接依赖）
    std::string connected_server_id_;
    
    // 线程管理
    std::thread heartbeat_thread_;
    std::thread reconnect_thread_;
    std::atomic<bool> heartbeat_running_{false};
    std::atomic<bool> reconnect_running_{false};
    
    // 重连管理
    std::atomic<uint32_t> reconnect_attempts_{0};
    std::atomic<uint64_t> last_reconnect_time_{0};
    
    // 统计信息
    mutable std::mutex stats_mutex_;
    uint32_t total_devices_registered_{0};
    uint32_t total_status_reports_{0};
    uint32_t total_data_reports_{0};
    uint32_t total_commands_processed_{0};
    uint32_t total_errors_{0};
    uint64_t client_start_time_{0};
    uint64_t last_heartbeat_time_{0};
};

} // namespace device_center
