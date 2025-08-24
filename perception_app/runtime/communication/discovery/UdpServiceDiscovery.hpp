#pragma once

#include "communication/interfaces/ITransport.hpp"
#include <asio.hpp>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <thread>
#include <array>

namespace perception {

/**
 * @brief UDP服务发现配置
 */
struct UdpServiceDiscoveryConfig {
    std::string local_address = "0.0.0.0";
    uint16_t discovery_port = 8081;
    uint32_t broadcast_interval = 2000; // ms
    uint32_t service_timeout = 10000; // ms
    bool enable_broadcast = true;
    bool enable_multicast = false;
    std::string multicast_address = "239.255.255.250";
    uint32_t max_services = 100;
};

/**
 * @brief UDP服务发现实现
 * 
 * 基于UDP的服务发现实现，支持广播和组播
 */
class UdpServiceDiscovery : public IServiceDiscovery {
public:
    explicit UdpServiceDiscovery(const UdpServiceDiscoveryConfig& config);
    ~UdpServiceDiscovery();

    void Stop() override;
    bool RegisterService(const EndpointIdentity& service_info) override;
    void UnregisterService() override;
    std::vector<EndpointIdentity> DiscoverServices(const std::string& service_name = "", ServiceDiscoveredCallback callback = nullptr) override;
    bool IsRunning() const override;

private:
    // 私有方法
    void EnsureReceiveStarted();
    void EnsureBroadcastStarted();
    void StartReceive();
    void StartBroadcastTimer();
    void HandleSend(const asio::error_code& ec, std::size_t bytes_transferred);
    void BroadcastServiceInfo();
    void ProcessDiscoveryMessage(const std::vector<uint8_t>& data, const asio::ip::udp::endpoint& sender);
    void ProcessServiceInfo(const EndpointIdentity& service_info);
    void CleanupExpiredServices();
    std::vector<uint8_t> SerializeServiceInfo(const EndpointIdentity& service_info);
    EndpointIdentity DeserializeServiceInfo(const std::vector<uint8_t>& data);
    uint64_t GetCurrentTimestamp();

private:
    UdpServiceDiscoveryConfig config_;
    
    // 接收侧（发现服务）
    std::shared_ptr<asio::io_context> recv_io_;
    std::unique_ptr<asio::ip::udp::socket> recv_socket_;
    asio::ip::udp::endpoint recv_remote_endpoint_;
    std::thread recv_thread_;
    std::atomic<bool> receiving_{false};
    
    // 广播侧（注册服务）
    std::shared_ptr<asio::io_context> bcast_io_;
    std::unique_ptr<asio::ip::udp::socket> bcast_socket_;
    std::unique_ptr<asio::steady_timer> bcast_timer_;
    std::thread bcast_thread_;
    std::atomic<bool> broadcasting_{false};
    
    // 服务管理
    EndpointIdentity local_service_info_;
    std::unordered_map<std::string, EndpointIdentity> discovered_services_;
    mutable std::mutex services_mutex_;
    
    // 回调函数
    ServiceDiscoveredCallback service_discovered_callback_;
    
    // 缓冲区
    std::array<uint8_t, 1024> recv_buffer_;
    
    // 统计信息
    std::atomic<uint32_t> services_discovered_{0};
    std::atomic<uint32_t> broadcasts_sent_{0};
    uint64_t start_time_{0};
};

} // namespace perception
