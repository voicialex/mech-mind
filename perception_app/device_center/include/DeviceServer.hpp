#pragma once

#include "DeviceTypes.hpp"
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <mutex>

// 前向声明
namespace perception {
    class Message;
    class CommunicationInterface;
}

namespace device_center {

/**
 * @brief 设备服务器 - 管理设备注册、发现和控制
 * 
 * 功能特性：
 * - 设备注册和管理
 * - 设备状态监控
 * - 设备控制命令处理
 * - 设备数据收集和分发
 * - 设备发现和查询
 */
class DeviceServer {
public:
    using DeviceHandlerPtr = DeviceHandler::Ptr;
    using EventHandlerPtr = DeviceEventHandler::Ptr;
    using CommandCallback = std::function<DeviceResponse(const DeviceCommand&)>;
    using StatusCallback = std::function<DeviceStatusInfo(const std::string&)>;

    /**
     * @brief 服务器配置
     */
    struct Config {
        std::string service_id = "device_server_001";
        std::string service_name = "Device Management Server";
        std::string local_address = "0.0.0.0";
        uint16_t local_port = 9090;
        uint16_t discovery_port = 9091;
        uint32_t max_devices = 100;
        uint32_t heartbeat_interval = 5000; // ms
        uint32_t device_timeout = 30000; // ms
        bool enable_auto_cleanup = true;
        uint32_t cleanup_interval = 60000; // ms
        std::string log_level = "INFO";
        std::string log_file = "device_server.log";
    };

public:
    explicit DeviceServer(const Config& config);
    DeviceServer(); // 默认构造函数
    ~DeviceServer();

    // 禁用拷贝
    DeviceServer(const DeviceServer&) = delete;
    DeviceServer& operator=(const DeviceServer&) = delete;

    /**
     * @brief 初始化设备服务器
     * @param config_path 配置文件路径
     * @return 是否初始化成功
     */
    bool Initialize(const std::string& config_path = "");

    /**
     * @brief 启动设备服务器
     * @return 是否启动成功
     */
    bool Start();

    /**
     * @brief 停止设备服务器
     */
    void Stop();

    /**
     * @brief 运行服务器循环
     */
    void Run();

    /**
     * @brief 清理资源
     */
    void Cleanup();

    /**
     * @brief 注册设备处理器
     * @param handler 设备处理器
     * @return 是否注册成功
     */
    bool RegisterDeviceHandler(DeviceHandlerPtr handler);

    /**
     * @brief 注册事件处理器
     * @param handler 事件处理器
     */
    void RegisterEventHandler(EventHandlerPtr handler);

    /**
     * @brief 注册设备命令回调
     * @param device_id 设备ID
     * @param callback 命令回调函数
     * @return 是否注册成功
     */
    bool RegisterCommandCallback(const std::string& device_id, CommandCallback callback);

    /**
     * @brief 注册设备状态回调
     * @param device_id 设备ID
     * @param callback 状态回调函数
     * @return 是否注册成功
     */
    bool RegisterStatusCallback(const std::string& device_id, StatusCallback callback);

    /**
     * @brief 注册设备
     * @param request 设备注册请求
     * @return 设备注册响应
     */
    DeviceRegisterResponse RegisterDevice(const DeviceRegisterRequest& request);

    /**
     * @brief 注销设备
     * @param device_id 设备ID
     * @return 是否注销成功
     */
    bool UnregisterDevice(const std::string& device_id);

    /**
     * @brief 发现设备
     * @param request 设备发现请求
     * @return 设备发现响应
     */
    DeviceDiscoveryResponse DiscoverDevices(const DeviceDiscoveryRequest& request);

    /**
     * @brief 获取设备信息
     * @param device_id 设备ID
     * @return 设备信息
     */
    DeviceInfo GetDeviceInfo(const std::string& device_id);

    /**
     * @brief 获取设备状态
     * @param device_id 设备ID
     * @return 设备状态
     */
    DeviceStatusInfo GetDeviceStatus(const std::string& device_id);

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
     * @brief 更新设备状态
     * @param device_id 设备ID
     * @param status 设备状态
     * @return 是否更新成功
     */
    bool UpdateDeviceStatus(const std::string& device_id, const DeviceStatusInfo& status);

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
     * @brief 获取所有设备
     * @return 设备列表
     */
    std::vector<DeviceInfo> GetAllDevices() const;

    /**
     * @brief 获取在线设备
     * @return 在线设备列表
     */
    std::vector<DeviceInfo> GetOnlineDevices() const;

    /**
     * @brief 检查设备是否在线
     * @param device_id 设备ID
     * @return 是否在线
     */
    bool IsDeviceOnline(const std::string& device_id) const;

    /**
     * @brief 检查设备是否存在
     * @param device_id 设备ID
     * @return 是否存在
     */
    bool HasDevice(const std::string& device_id) const;

    /**
     * @brief 获取服务器统计信息
     * @return 统计信息JSON
     */
    nlohmann::json GetServerStatistics() const;

    /**
     * @brief 获取服务器配置
     * @return 服务器配置
     */
    Config GetConfig() const;

    /**
     * @brief 检查服务器是否运行
     * @return 是否运行
     */
    bool IsRunning() const;

    /**
     * @brief 检查服务器是否已初始化
     * @return 是否已初始化
     */
    bool IsInitialized() const;

private:
    // 私有方法
    void LoadConfig(const std::string& config_path);
    void InitializeCommunication();
    void StartHeartbeatMonitor();
    void StopHeartbeatMonitor();
    void ProcessHeartbeat();
    void CleanupOfflineDevices();
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
    bool ValidateDeviceRegistration(const DeviceRegisterRequest& request);
    std::string GenerateDeviceId(const DeviceRegisterRequest& request);
    
    // 通信处理方法
    void HandleDeviceMessage(const std::shared_ptr<perception::Message>& message);
    void HandleClientConnection(const std::string& service_id, bool connected);
    void HandleCommunicationError(const std::string& service_id, uint16_t error_code);

private:
    Config config_;
    std::atomic<bool> initialized_{false};
    std::atomic<bool> running_{false};
    
    // 设备管理
    std::unordered_map<std::string, DeviceInfo> devices_;
    std::unordered_map<std::string, DeviceStatusInfo> device_status_;
    std::unordered_map<std::string, std::string> device_clients_; // device_id -> client_id
    std::unordered_map<std::string, std::vector<std::string>> client_devices_; // client_id -> device_ids
    mutable std::mutex devices_mutex_;
    
    // 处理器管理
    DeviceHandlerPtr device_handler_;
    std::unordered_map<std::string, CommandCallback> command_callbacks_;
    std::unordered_map<std::string, StatusCallback> status_callbacks_;
    EventHandlerPtr event_handler_;
    mutable std::mutex handlers_mutex_;
    
    // 通信管理
    struct CommunicationDeleter {
        void operator()(perception::CommunicationInterface* ptr) const;
    };
    std::unique_ptr<perception::CommunicationInterface, CommunicationDeleter> comm_interface_; // 通信接口
    
    // 线程管理
    std::thread heartbeat_thread_;
    std::thread cleanup_thread_;
    std::atomic<bool> heartbeat_running_{false};
    std::atomic<bool> cleanup_running_{false};
    
    // 统计信息
    mutable std::mutex stats_mutex_;
    uint32_t total_devices_registered_{0};
    uint32_t total_devices_online_{0};
    uint32_t total_commands_processed_{0};
    uint32_t total_data_received_{0};
    uint32_t total_errors_{0};
    uint64_t server_start_time_{0};
};

} // namespace device_center
