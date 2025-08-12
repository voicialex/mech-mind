#pragma once

#include "DeviceTypes.hpp"
#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace device_center {

/**
 * @brief 设备通信适配器 - 薄适配层
 * 
 * 功能：
 * - 将 device_center 的通信需求适配到 perception_app/communication
 * - 保持 device_center 的接口不变
 * - 提供可选的通信功能，不影响 device_center 的独立性
 */
class DeviceCommunicationAdapter {
public:
    using MessageCallback = std::function<void(const std::vector<uint8_t>&)>;
    using ConnectionCallback = std::function<void(bool)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    /**
     * @brief 适配器配置
     */
    struct Config {
        std::string service_id;
        std::string service_name;
        std::string local_address = "0.0.0.0";
        uint16_t local_port = 0;
        std::string remote_address = "127.0.0.1";
        uint16_t remote_port = 0;
        bool enable_auto_reconnect = true;
        uint32_t reconnect_interval = 5000; // ms
        uint32_t heartbeat_interval = 5000; // ms
        std::string log_level = "INFO";
    };

public:
    explicit DeviceCommunicationAdapter(const Config& config);
    ~DeviceCommunicationAdapter();

    // 禁用拷贝
    DeviceCommunicationAdapter(const DeviceCommunicationAdapter&) = delete;
    DeviceCommunicationAdapter& operator=(const DeviceCommunicationAdapter&) = delete;

    /**
     * @brief 初始化适配器
     * @return 是否初始化成功
     */
    bool Initialize();

    /**
     * @brief 启动适配器
     * @return 是否启动成功
     */
    bool Start();

    /**
     * @brief 停止适配器
     */
    void Stop();

    /**
     * @brief 清理资源
     */
    void Cleanup();

    /**
     * @brief 发送消息
     * @param message_data 消息数据
     * @return 是否发送成功
     */
    bool SendMessage(const std::vector<uint8_t>& message_data);

    /**
     * @brief 发送请求并等待响应
     * @param request_data 请求数据
     * @param response_data 响应数据（输出）
     * @param timeout_ms 超时时间（毫秒）
     * @return 是否成功
     */
    bool SendRequest(const std::vector<uint8_t>& request_data, 
                    std::vector<uint8_t>& response_data, 
                    uint32_t timeout_ms = 5000);

    /**
     * @brief 广播消息
     * @param message_data 消息数据
     * @return 是否广播成功
     */
    bool BroadcastMessage(const std::vector<uint8_t>& message_data);

    /**
     * @brief 注册消息回调
     * @param callback 消息回调函数
     */
    void RegisterMessageCallback(MessageCallback callback);

    /**
     * @brief 注册连接状态回调
     * @param callback 连接状态回调函数
     */
    void RegisterConnectionCallback(ConnectionCallback callback);

    /**
     * @brief 注册错误回调
     * @param callback 错误回调函数
     */
    void RegisterErrorCallback(ErrorCallback callback);

    /**
     * @brief 检查是否已连接
     * @return 是否已连接
     */
    bool IsConnected() const;

    /**
     * @brief 获取连接状态信息
     * @return 连接状态JSON
     */
    nlohmann::json GetConnectionStatus() const;

    /**
     * @brief 获取统计信息
     * @return 统计信息JSON
     */
    nlohmann::json GetStatistics() const;

private:
    // 私有实现
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace device_center
