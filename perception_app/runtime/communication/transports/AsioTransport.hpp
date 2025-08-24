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
 * @brief ASIO传输层实现
 * 
 * 基于ASIO库的传输层实现，支持TCP和UDP
 */
class AsioTransport : public ITransport {
public:
    // 允许检查器访问内部统计
    friend class TransportInspector;
    explicit AsioTransport(const EndpointIdentity& config);
    ~AsioTransport();

    // ITransport接口实现
    bool Initialize() override;
    bool Start() override;
    void Stop() override;
    bool Connect(const std::string& service_id, const std::string& address, uint16_t port) override;
    void Disconnect(const std::string& service_id) override;
    bool SendMessage(const std::string& target_id, const std::vector<uint8_t>& data) override;
    bool BroadcastMessage(const std::vector<uint8_t>& data, const std::string& target_filter = "") override;
    void RegisterEventHandler(EventHandler::Ptr handler) override;
    ConnectionInfo GetConnectionInfo(const std::string& service_id) const override;
    std::vector<ConnectionInfo> GetAllConnections() const override;
    bool IsConnected(const std::string& service_id) const override;
    bool IsRunning() const override;

private:
    // TCP连接类
    class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
    public:
        TcpConnection(asio::ip::tcp::socket socket, const std::string& service_id, AsioTransport* owner);
        
        void Start();
        void Close();
        bool SendMessage(const std::vector<uint8_t>& data);
        // 在accept后设置连接ID，确保上层回调携带正确的endpoint_id
        void SetServiceId(const std::string& service_id);
        bool IsConnected() const;
        ConnectionInfo GetConnectionInfo() const;
        ConnectionInfo& GetConnectionInfoRef();
        
        asio::ip::tcp::socket& GetSocket() { return socket_; }
        const std::string& GetServiceId() const { return service_id_; }

    private:
        void StartRead();
        
        asio::ip::tcp::socket socket_;
        std::string service_id_;
        AsioTransport* owner_{nullptr};
        mutable ConnectionInfo connection_info_;
        std::array<uint8_t, 8192> read_buffer_;
        std::vector<uint8_t> write_buffer_;
        std::mutex write_mutex_;
        uint32_t message_length_{0};
        std::vector<uint8_t> message_data_;
    };

    // UDP服务类
    class UdpService {
    public:
        UdpService(asio::ip::udp::socket socket);
        
        void Start();
        void Stop();
        bool SendMessage(const asio::ip::udp::endpoint& endpoint, const std::vector<uint8_t>& data);
        bool BroadcastMessage(const std::vector<uint8_t>& data);

    private:
        void StartReceive();
        void HandleReceive(const asio::error_code& ec, std::size_t bytes_transferred);
        void HandleSend(const asio::error_code& ec, std::size_t bytes_transferred);
        
        asio::ip::udp::socket socket_;
        asio::ip::udp::endpoint remote_endpoint_;
        std::array<uint8_t, 8192> recv_buffer_;
    };

    // 私有方法
    void StartAccept();
    void StartIoContext();
    void StopIoContext();
    
    // 连接管理
    void AddConnection(const std::string& service_id, std::shared_ptr<TcpConnection> connection);
    void RemoveConnection(const std::string& service_id);
    std::shared_ptr<TcpConnection> GetConnection(const std::string& service_id) const;

private:
    EndpointIdentity config_;
    std::atomic<bool> running_{false};
    
    // ASIO相关
    std::shared_ptr<asio::io_context> io_context_;
    std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> work_guard_;
    std::thread io_thread_;
    
    // TCP相关
    std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;
    std::unordered_map<std::string, std::shared_ptr<TcpConnection>> connections_;
    mutable std::mutex connections_mutex_;
    
    // UDP相关
    std::unique_ptr<UdpService> udp_service_;
    
    // 事件处理器
    EventHandler::Ptr event_handler_;
    
    // 统计信息
    std::atomic<uint32_t> messages_sent_{0};
    std::atomic<uint32_t> messages_received_{0};
    std::atomic<uint32_t> connection_errors_{0};
    uint64_t start_time_{0};
    
    // 静态成员
    static uint32_t connection_counter_;
};

} // namespace perception
