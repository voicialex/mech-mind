#include "CommunicationManager.hpp"
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <algorithm>

namespace perception {

// UDP发现服务实现
class UdpDiscoveryService {
public:
    enum class Mode {
        Server,  // 服务器模式：绑定端口进行广播
        Client   // 客户端模式：监听端口接收广播
    };
    
    using ServiceDiscoveredCallback = std::function<void(const CommunicationManager::ServiceInfo&)>;
    using BroadcastMessageCallback = std::function<void(const std::vector<uint8_t>&)>;
    
    UdpDiscoveryService(
        std::shared_ptr<asio::io_context> io_context,
        uint16_t port,
        ServiceDiscoveredCallback discovered_callback,
        BroadcastMessageCallback broadcast_callback,
        Mode mode = Mode::Server
    ) : io_context_(io_context),
        port_(port),
        mode_(mode),
        discovered_callback_(std::move(discovered_callback)),
        broadcast_callback_(std::move(broadcast_callback)),
        socket_(*io_context) {
        
        if (mode_ == Mode::Server) {
            // 服务器模式：绑定到指定端口
            socket_.open(asio::ip::udp::v4());
            socket_.set_option(asio::socket_base::reuse_address(true));
            socket_.set_option(asio::socket_base::broadcast(true)); // 启用广播权限
            socket_.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), port));
        } else {
            // 客户端模式：绑定到发现端口以接收广播
            socket_.open(asio::ip::udp::v4());
            socket_.set_option(asio::socket_base::reuse_address(true));
            socket_.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), port));
            std::cout << "[ASIO] 客户端模式：绑定到端口 " << port << " 接收广播" << std::endl;
        }
        
        std::cout << "[ASIO] UdpDiscoveryService 初始化完成，端口: " << port 
                  << ", 模式: " << (mode_ == Mode::Server ? "服务器" : "客户端") << std::endl;
    }
    
    void Start() {
        std::cout << "[ASIO] UdpDiscoveryService 开始启动，端口: " << port_ << std::endl;
        StartReceive();
        
        // 只有服务器模式才启动广播定时器
        if (mode_ == Mode::Server) {
            StartBroadcastTimer();
        }
        
        std::cout << "[ASIO] UdpDiscoveryService 启动完成" << std::endl;
    }
    
    void Stop() {
        socket_.close();
    }
    
    void StartBroadcast(const CommunicationManager::ServiceInfo& service_info) {
        service_info_ = service_info;
        
        // 只有服务器模式才启动广播
        if (mode_ == Mode::Server) {
            std::cout << "[ASIO] 开始广播服务信息: " << service_info.service_id 
                      << " (" << service_info.service_name << ") 端口: " << service_info.port << std::endl;
            StartBroadcastTimer();
        } else {
            std::cout << "[ASIO] 客户端模式：不启动广播，只监听服务发现" << std::endl;
        }
    }
    
    void StopBroadcast() {
        broadcast_timer_.cancel();
    }
    
    void BroadcastMessage(const std::vector<uint8_t>& data, const std::string& service_name) {
        nlohmann::json message;
        message["type"] = "broadcast";
        message["service_name"] = service_name;
        message["data"] = data;
        
        std::string json_str = message.dump();
        std::vector<uint8_t> message_data(json_str.begin(), json_str.end());
        
        BroadcastUdpMessage(message_data);
    }
    
private:
    void StartReceive() {
        std::cout << "[ASIO] 开始监听UDP消息，端口: " << port_ << std::endl;
        socket_.async_receive_from(
            asio::buffer(recv_buffer_),
            remote_endpoint_,
            [this](const asio::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    std::cout << "[ASIO] 收到 " << bytes_transferred << " 字节的UDP数据" << std::endl;
                    HandleReceivedMessage(bytes_transferred);
                } else {
                    std::cout << "[ASIO] UDP接收错误: " << ec.message() << std::endl;
                }
                StartReceive();
            }
        );
    }
    
    void HandleReceivedMessage(std::size_t bytes_transferred) {
        std::string message(recv_buffer_.begin(), recv_buffer_.begin() + bytes_transferred);
        
        std::cout << "[ASIO] 收到UDP消息: " << message << std::endl;
        
        try {
            nlohmann::json json_msg = nlohmann::json::parse(message);
            
            if (json_msg["type"] == "service_discovery") {
                std::cout << "[ASIO] 处理服务发现消息" << std::endl;
                HandleServiceDiscovery(json_msg);
            } else if (json_msg["type"] == "broadcast") {
                std::cout << "[ASIO] 处理广播消息" << std::endl;
                HandleBroadcastMessage(json_msg);
            }
        } catch (const std::exception& e) {
            std::cout << "[ASIO] UDP消息解析错误: " << e.what() << std::endl;
        }
    }
    
    void HandleServiceDiscovery(const nlohmann::json& json_msg) {
        CommunicationManager::ServiceInfo service;
        service.service_id = json_msg["service_id"];
        service.service_name = json_msg["service_name"];
        service.address = remote_endpoint_.address().to_string();
        service.port = json_msg["port"];
        service.version = json_msg.value("version", "1.0.0");
        service.last_heartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        service.is_online = true;
        
        // 发现服务后，如果本实例是客户端模式，则可尝试立即连接
        if (mode_ == Mode::Client) {
            std::cout << "[ASIO] (客户端) 自动连接发现的服务: " << service.service_id << std::endl;
        }

        std::cout << "[ASIO] 发现服务: " << service.service_id 
                  << " (" << service.service_name << ") 地址: " << service.address 
                  << ":" << service.port << std::endl;
        
        if (discovered_callback_) {
            discovered_callback_(service);
        }
    }
    
    void HandleBroadcastMessage(const nlohmann::json& json_msg) {
        if (json_msg.contains("data") && broadcast_callback_) {
            auto data = json_msg["data"].get<std::vector<uint8_t>>();
            broadcast_callback_(data);
        }
    }
    
    void StartBroadcastTimer() {
        broadcast_timer_.expires_after(std::chrono::seconds(2));
        broadcast_timer_.async_wait([this](const asio::error_code& ec) {
            if (!ec) {
                BroadcastServiceInfo();
                StartBroadcastTimer();
            }
        });
    }
    
    void BroadcastServiceInfo() {
        if (service_info_.service_id.empty()) {
            std::cout << "[ASIO] 服务信息为空，跳过广播" << std::endl;
            return;
        }
        
        nlohmann::json discovery_msg;
        discovery_msg["type"] = "service_discovery";
        discovery_msg["service_id"] = service_info_.service_id;
        discovery_msg["service_name"] = service_info_.service_name;
        discovery_msg["port"] = service_info_.port;
        discovery_msg["version"] = service_info_.version;
        discovery_msg["capabilities"] = service_info_.capabilities;
        
        std::string json_str = discovery_msg.dump();
        std::vector<uint8_t> message_data(json_str.begin(), json_str.end());
        
        std::cout << "[ASIO] 广播服务发现消息: " << service_info_.service_id << std::endl;
        BroadcastUdpMessage(message_data);
    }
    
    void BroadcastUdpMessage(const std::vector<uint8_t>& data) {
        asio::ip::udp::endpoint broadcast_endpoint(
            asio::ip::address_v4::broadcast(), port_);
        
        std::cout << "[ASIO] 发送UDP广播到: " << broadcast_endpoint.address().to_string() 
                  << ":" << broadcast_endpoint.port() << std::endl;
        
        socket_.async_send_to(
            asio::buffer(data),
            broadcast_endpoint,
            [](const asio::error_code& ec, std::size_t) {
                if (ec) {
                    std::cout << "[ASIO] UDP广播发送错误: " << ec.message() << std::endl;
                } else {
                    std::cout << "[ASIO] UDP广播发送成功" << std::endl;
                }
            }
        );
    }
    
    std::shared_ptr<asio::io_context> io_context_;
    uint16_t port_;
    Mode mode_;
    ServiceDiscoveredCallback discovered_callback_;
    BroadcastMessageCallback broadcast_callback_;
    CommunicationManager::ServiceInfo service_info_;
    
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_endpoint_;
    std::array<uint8_t, 1024> recv_buffer_;
    
    asio::steady_timer broadcast_timer_{*io_context_};
};

// TCP连接实现
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using MessageReceivedCallback = std::function<void(const std::string&, const std::vector<uint8_t>&)>;
    using ConnectionChangedCallback = std::function<void(const std::string&, bool)>;
    using ConnectionErrorCallback = std::function<void(const std::string&, const asio::error_code&)>;
    
    TcpConnection(
        std::shared_ptr<asio::io_context> io_context,
        const std::string& service_id,
        MessageReceivedCallback message_callback,
        ConnectionChangedCallback connection_callback,
        ConnectionErrorCallback error_callback
    ) : io_context_(io_context),
        service_id_(service_id),
        message_callback_(std::move(message_callback)),
        connection_callback_(std::move(connection_callback)),
        error_callback_(std::move(error_callback)),
        socket_(*io_context) {
    }
    
    asio::ip::tcp::socket& GetSocket() { return socket_; }
    
    void Connect(const std::string& address, uint16_t port) {
        asio::ip::tcp::resolver resolver(*io_context_);
        auto endpoints = resolver.resolve(address, std::to_string(port));
        
        asio::async_connect(socket_, endpoints,
            [this](const asio::error_code& ec, const asio::ip::tcp::endpoint&) {
                if (!ec) {
                    connection_info_.state = CommunicationManager::ConnectionState::Connected;
                    connection_info_.remote_address = socket_.remote_endpoint().address().to_string();
                    connection_info_.remote_port = socket_.remote_endpoint().port();
                    connection_info_.last_activity = GetCurrentTime();
                    
                    if (connection_callback_) {
                        connection_callback_(service_id_, true);
                    }
                    
                    StartRead();
                } else {
                    connection_info_.state = CommunicationManager::ConnectionState::Error;
                    if (error_callback_) {
                        error_callback_(service_id_, ec);
                    }
                }
            }
        );
    }
    
    void Start() {
        connection_info_.state = CommunicationManager::ConnectionState::Connected;
        connection_info_.remote_address = socket_.remote_endpoint().address().to_string();
        connection_info_.remote_port = socket_.remote_endpoint().port();
        connection_info_.last_activity = GetCurrentTime();
        
        if (connection_callback_) {
            connection_callback_(service_id_, true);
        }
        
        StartRead();
    }
    
    void Close() {
        if (socket_.is_open()) {
            asio::error_code ec;
            socket_.close(ec);
        }
        connection_info_.state = CommunicationManager::ConnectionState::Disconnected;
        
        if (connection_callback_) {
            connection_callback_(service_id_, false);
        }
    }
    
    bool SendMessage(const std::vector<uint8_t>& data) {
        if (connection_info_.state != CommunicationManager::ConnectionState::Connected) {
            return false;
        }
        
        // 简单的消息格式：长度 + 数据
        uint32_t length = static_cast<uint32_t>(data.size());
        std::vector<uint8_t> message;
        message.insert(message.end(), 
            reinterpret_cast<uint8_t*>(&length),
            reinterpret_cast<uint8_t*>(&length) + sizeof(length));
        message.insert(message.end(), data.begin(), data.end());
        
        asio::async_write(socket_, asio::buffer(message),
            [this](const asio::error_code& ec, std::size_t) {
                if (ec) {
                    if (error_callback_) {
                        error_callback_(service_id_, ec);
                    }
                } else {
                    connection_info_.message_count++;
                    connection_info_.last_activity = GetCurrentTime();
                }
            }
        );
        
        return true;
    }
    
    std::vector<uint8_t> SendRequest(const std::vector<uint8_t>& data, uint32_t timeout_ms) {
        // 简化实现：同步发送请求
        if (connection_info_.state != CommunicationManager::ConnectionState::Connected) {
            return {};
        }
        
        if (SendMessage(data)) {
            // 等待响应（简化实现）
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
            return {}; // 实际应该返回接收到的响应
        }
        
        return {};
    }
    
    CommunicationManager::ConnectionInfo GetConnectionInfo() const {
        return connection_info_;
    }
    
    bool IsConnected() const {
        return connection_info_.state == CommunicationManager::ConnectionState::Connected;
    }
    
    void ScheduleReconnect() {
        // 简化实现：立即重连
        if (connection_info_.state != CommunicationManager::ConnectionState::Connected) {
            connection_info_.reconnect_attempts++;
            // 实际应该实现延迟重连逻辑
        }
    }
    
private:
    void StartRead() {
        // 先读取消息长度
        asio::async_read(socket_, asio::buffer(&message_length_, sizeof(message_length_)),
            [this](const asio::error_code& ec, std::size_t) {
                if (!ec) {
                    message_data_.resize(message_length_);
                    // 读取消息数据
                    asio::async_read(socket_, asio::buffer(message_data_),
                        [this](const asio::error_code& ec, std::size_t) {
                            if (!ec) {
                                connection_info_.last_activity = GetCurrentTime();
                                if (message_callback_) {
                                    message_callback_(service_id_, message_data_);
                                }
                                StartRead();
                            } else {
                                if (error_callback_) {
                                    error_callback_(service_id_, ec);
                                }
                            }
                        }
                    );
                } else {
                    if (error_callback_) {
                        error_callback_(service_id_, ec);
                    }
                }
            }
        );
    }
    
    uint64_t GetCurrentTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
    std::shared_ptr<asio::io_context> io_context_;
    std::string service_id_;
    MessageReceivedCallback message_callback_;
    ConnectionChangedCallback connection_callback_;
    ConnectionErrorCallback error_callback_;
    
    asio::ip::tcp::socket socket_;
    CommunicationManager::ConnectionInfo connection_info_;
    
    uint32_t message_length_{0};
    std::vector<uint8_t> message_data_;
};

// TCP连接管理实现
class TcpConnectionManager {
public:
    using MessageReceivedCallback = std::function<void(const std::string&, const std::vector<uint8_t>&)>;
    using ConnectionChangedCallback = std::function<void(const std::string&, bool)>;
    using ConnectionErrorCallback = std::function<void(const std::string&, const asio::error_code&)>;
    
    TcpConnectionManager(
        std::shared_ptr<asio::io_context> io_context,
        uint16_t port,
        MessageReceivedCallback message_callback,
        ConnectionChangedCallback connection_callback,
        ConnectionErrorCallback error_callback
    ) : io_context_(io_context),
        port_(port),
        message_callback_(std::move(message_callback)),
        connection_callback_(std::move(connection_callback)),
        error_callback_(std::move(error_callback)),
        acceptor_(*io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
        std::cout << "[ASIO] TcpConnectionManager 初始化完成，端口: " << port << std::endl;
    }
    
    void Start() {
        std::cout << "[ASIO] TcpConnectionManager 开始启动，端口: " << port_ << std::endl;
        StartAccept();
        std::cout << "[ASIO] TcpConnectionManager 启动完成，开始监听端口: " << port_ << std::endl;
    }
    
    void Stop() {
        acceptor_.close();
        for (auto& [id, connection] : connections_) {
            connection->Close();
        }
        connections_.clear();
    }
    
    bool ConnectToService(const std::string& service_id, const std::string& address, uint16_t port) {
        auto connection = std::make_shared<TcpConnection>(
            io_context_, service_id, message_callback_, connection_callback_, error_callback_);
        
        connection->Connect(address, port);
        connections_[service_id] = connection;
        
        return true;
    }
    
    void DisconnectFromService(const std::string& service_id) {
        auto it = connections_.find(service_id);
        if (it != connections_.end()) {
            it->second->Close();
            connections_.erase(it);
        }
    }
    
    bool SendMessage(const std::string& service_id, const std::vector<uint8_t>& data) {
        auto it = connections_.find(service_id);
        if (it != connections_.end()) {
            return it->second->SendMessage(data);
        }
        return false;
    }
    
    std::vector<uint8_t> SendRequest(const std::string& service_id, const std::vector<uint8_t>& data, uint32_t timeout_ms) {
        auto it = connections_.find(service_id);
        if (it != connections_.end()) {
            return it->second->SendRequest(data, timeout_ms);
        }
        return {};
    }
    
    CommunicationManager::ConnectionInfo GetConnectionInfo(const std::string& service_id) const {
        auto it = connections_.find(service_id);
        if (it != connections_.end()) {
            return it->second->GetConnectionInfo();
        }
        return {};
    }
    
    std::vector<CommunicationManager::ConnectionInfo> GetAllConnections() const {
        std::vector<CommunicationManager::ConnectionInfo> result;
        for (const auto& [id, connection] : connections_) {
            result.push_back(connection->GetConnectionInfo());
        }
        return result;
    }
    
    bool IsServiceOnline(const std::string& service_id) const {
        auto it = connections_.find(service_id);
        return it != connections_.end() && it->second->IsConnected();
    }
    
    void ScheduleReconnect(const std::string& service_id) {
        auto it = connections_.find(service_id);
        if (it != connections_.end()) {
            it->second->ScheduleReconnect();
        }
    }
    
private:
    void StartAccept() {
        std::cout << "[ASIO] 开始接受连接，端口: " << port_ << std::endl;
        auto connection = std::make_shared<TcpConnection>(
            io_context_, "", message_callback_, connection_callback_, error_callback_);
        
        acceptor_.async_accept(
            connection->GetSocket(),
            [this, connection](const asio::error_code& ec) {
                if (!ec) {
                    std::cout << "[ASIO] 接受新连接成功" << std::endl;
                    connection->Start();
                    // 为接受的连接生成ID
                    std::string connection_id = "accepted_" + std::to_string(connection_counter_++);
                    connections_[connection_id] = connection;
                } else {
                    std::cout << "[ASIO] 接受连接失败: " << ec.message() << std::endl;
                }
                StartAccept();
            }
        );
    }
    
    std::shared_ptr<asio::io_context> io_context_;
    uint16_t port_;
    MessageReceivedCallback message_callback_;
    ConnectionChangedCallback connection_callback_;
    ConnectionErrorCallback error_callback_;
    
    asio::ip::tcp::acceptor acceptor_;
    std::unordered_map<std::string, std::shared_ptr<TcpConnection>> connections_;
    uint32_t connection_counter_{0};
};

// CommunicationManager实现
CommunicationManager::CommunicationManager(const Config &config) 
    : config_(config), 
      io_context_(std::make_shared<asio::io_context>()),
      work_guard_(asio::make_work_guard(*io_context_)) {
}

CommunicationManager::CommunicationManager() 
    : config_(), 
      io_context_(std::make_shared<asio::io_context>()),
      work_guard_(asio::make_work_guard(*io_context_)) {
}

CommunicationManager::~CommunicationManager() {
    Stop();
}

bool CommunicationManager::Start() {
    if (running_) return true;
    
    try {
        std::cout << "[ASIO] CommunicationManager 开始启动..." << std::endl;
        std::cout << "[ASIO] 配置信息 - 本地端口: " << config_.local_port << ", 发现端口: " << config_.discovery_port << std::endl;
        
        // 启动IO上下文线程
        io_thread_ = std::thread([this]() {
            std::cout << "[ASIO] IO上下文线程启动" << std::endl;
            io_context_->run();
            std::cout << "[ASIO] IO上下文线程结束" << std::endl;
        });
        
        // 启动UDP发现服务
        StartDiscoveryService();
        
        // 启动TCP连接管理
        StartConnectionManager();
        
        running_ = true;
        std::cout << "[ASIO] CommunicationManager 启动成功" << std::endl;
  return true;
    } catch (const std::exception& e) {
        std::cerr << "[ASIO] CommunicationManager启动失败: " << e.what() << std::endl;
        return false;
    }
}

void CommunicationManager::Stop() {
    if (!running_) return;
    
    running_ = false;
    
    // 停止工作守卫
    work_guard_.reset();
    
    // 停止IO上下文
    if (io_context_) {
        io_context_->stop();
    }
    
    // 等待IO线程结束
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
    
    // 清理连接
    connections_.clear();
    discovered_services_.clear();
}

bool CommunicationManager::RegisterService(const ServiceInfo &service_info) {
    local_service_info_ = service_info;
    
    // 启动服务广播
    if (discovery_service_) {
        static_cast<UdpDiscoveryService*>(discovery_service_.get())->StartBroadcast(service_info);
    }
    
  return true;
}

void CommunicationManager::UnregisterService() {
    if (discovery_service_) {
        static_cast<UdpDiscoveryService*>(discovery_service_.get())->StopBroadcast();
    }
}

std::vector<CommunicationManager::ServiceInfo> CommunicationManager::DiscoverServices(const std::string &service_name) {
    std::lock_guard<std::mutex> lock(services_mutex_);
    std::vector<ServiceInfo> result;
    
    std::cout << "[ASIO] 查找服务: '" << service_name << "', 当前发现的服务数量: " << discovered_services_.size() << std::endl;
    
    for (const auto& [id, service] : discovered_services_) {
        std::cout << "[ASIO] 检查服务: " << service.service_id << " (" << service.service_name << ")" << std::endl;
        if (service_name.empty() || service.service_name == service_name) {
            result.push_back(service);
            std::cout << "[ASIO] 匹配服务: " << service.service_id << std::endl;
        }
    }
    
    std::cout << "[ASIO] 返回匹配的服务数量: " << result.size() << std::endl;
    return result;
}

bool CommunicationManager::ConnectToService(const std::string &service_id) {
    std::lock_guard<std::mutex> lock(services_mutex_);
    
    std::cout << "[ASIO] 尝试连接到服务: " << service_id << std::endl;
    
    auto it = discovered_services_.find(service_id);
    if (it == discovered_services_.end()) {
        std::cout << "[ASIO] 服务未找到: " << service_id << std::endl;
        return false;
    }
    
    const auto& service = it->second;
    std::cout << "[ASIO] 找到服务，尝试连接到: " << service.address << ":" << service.port << std::endl;
    
    // 创建连接
    if (connection_manager_) {
        bool result = static_cast<TcpConnectionManager*>(connection_manager_.get())->ConnectToService(service_id, service.address, service.port);
        std::cout << "[ASIO] TCP连接结果: " << (result ? "成功" : "失败") << std::endl;
        return result;
    }
    
    std::cout << "[ASIO] 连接管理器未初始化" << std::endl;
    return false;
}

void CommunicationManager::DisconnectFromService(const std::string &service_id) {
    if (connection_manager_) {
        static_cast<TcpConnectionManager*>(connection_manager_.get())->DisconnectFromService(service_id);
    }
}

bool CommunicationManager::SendMessage(const std::string &target_id, const std::vector<uint8_t> &data) {
    if (connection_manager_) {
        return static_cast<TcpConnectionManager*>(connection_manager_.get())->SendMessage(target_id, data);
    }
    return false;
}

std::vector<uint8_t> CommunicationManager::SendRequest(const std::string &target_id, const std::vector<uint8_t> &data, uint32_t timeout_ms) {
    if (connection_manager_) {
        return static_cast<TcpConnectionManager*>(connection_manager_.get())->SendRequest(target_id, data, timeout_ms);
    }
  return {};
}

void CommunicationManager::BroadcastMessage(const std::vector<uint8_t> &data, const std::string &service_name) {
    if (discovery_service_) {
        static_cast<UdpDiscoveryService*>(discovery_service_.get())->BroadcastMessage(data, service_name);
    }
}

void CommunicationManager::RegisterMessageHandler(MessageHandler handler) {
    message_handler_ = std::move(handler);
}

void CommunicationManager::RegisterConnectionHandler(ConnectionHandler handler) {
    connection_handler_ = std::move(handler);
}

void CommunicationManager::RegisterErrorHandler(ErrorHandler handler) {
    error_handler_ = std::move(handler);
}

CommunicationManager::ConnectionInfo CommunicationManager::GetConnectionInfo(const std::string &service_id) const {
    if (connection_manager_) {
        return static_cast<TcpConnectionManager*>(connection_manager_.get())->GetConnectionInfo(service_id);
    }
  return {};
}

std::vector<CommunicationManager::ConnectionInfo> CommunicationManager::GetAllConnections() const {
    if (connection_manager_) {
        return static_cast<TcpConnectionManager*>(connection_manager_.get())->GetAllConnections();
    }
  return {};
}

bool CommunicationManager::IsServiceOnline(const std::string &service_id) const {
    if (connection_manager_) {
        return static_cast<TcpConnectionManager*>(connection_manager_.get())->IsServiceOnline(service_id);
    }
  return false;
}

CommunicationManager::ServiceInfo CommunicationManager::GetLocalServiceInfo() const {
    return local_service_info_;
}

std::string CommunicationManager::GetStatistics() const {
    nlohmann::json stats;
    stats["running"] = running_.load();
    stats["local_service"] = {
        {"id", local_service_info_.service_id},
        {"name", local_service_info_.service_name},
        {"address", local_service_info_.address},
        {"port", local_service_info_.port}
    };
    
    if (connection_manager_) {
        auto connections = static_cast<TcpConnectionManager*>(connection_manager_.get())->GetAllConnections();
        stats["connections"] = connections.size();
    }
    
    std::lock_guard<std::mutex> lock(services_mutex_);
    stats["discovered_services"] = discovered_services_.size();
    
    return stats.dump(2);
}

void CommunicationManager::ScheduleReconnect(const std::string &service_id) {
    if (connection_manager_) {
        static_cast<TcpConnectionManager*>(connection_manager_.get())->ScheduleReconnect(service_id);
    }
}

// 私有方法实现
void CommunicationManager::StartDiscoveryService() {
    std::cout << "[ASIO] 启动UDP发现服务，端口: " << config_.discovery_port << std::endl;
    
    // 根据配置选择模式
    UdpDiscoveryService::Mode mode = config_.is_server ? 
        UdpDiscoveryService::Mode::Server : UdpDiscoveryService::Mode::Client;
    
    std::cout << "[ASIO] 使用模式: " << (mode == UdpDiscoveryService::Mode::Server ? "服务器" : "客户端") << std::endl;
    
    // 尝试启动UDP发现服务，如果端口被占用则尝试其他端口
    uint16_t port = config_.discovery_port;
    int max_attempts = 10;
    
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        try {
            auto discovery_service = std::make_unique<UdpDiscoveryService>(
                io_context_, 
                port,
                [this](const ServiceInfo& service) {
                    OnServiceDiscovered(service);
                },
                [this](const std::vector<uint8_t>& data) {
                    OnBroadcastMessageReceived(data);
                },
                mode
            );
            
            auto deleter = [](void* ptr) {
                delete static_cast<UdpDiscoveryService*>(ptr);
            };
            
            discovery_service_ = std::unique_ptr<void, std::function<void(void*)>>(
                discovery_service.release(), deleter);
            
            static_cast<UdpDiscoveryService*>(discovery_service_.get())->Start();
            std::cout << "[ASIO] UDP发现服务启动完成，端口: " << port << std::endl;
            return;
        } catch (const std::exception& e) {
            std::cout << "[ASIO] 端口 " << port << " 被占用，尝试端口 " << (port + 1) << std::endl;
            port++;
        }
    }
    
    std::cerr << "[ASIO] 无法找到可用的UDP发现端口" << std::endl;
}

void CommunicationManager::StartConnectionManager() {
    std::cout << "[ASIO] 启动TCP连接管理器，端口: " << config_.local_port << std::endl;
    
    // 尝试启动TCP连接管理器，如果端口被占用则尝试其他端口
    uint16_t port = config_.local_port;
    int max_attempts = 10;
    
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        try {
            auto connection_manager = std::make_unique<TcpConnectionManager>(
                io_context_,
                port,
                [this](const std::string& service_id, const std::vector<uint8_t>& data) {
                    OnMessageReceived(service_id, data);
                },
                [this](const std::string& service_id, bool connected) {
                    OnConnectionChanged(service_id, connected);
                },
                [this](const std::string& service_id, const asio::error_code& ec) {
                    OnConnectionError(service_id, ec);
                }
            );
            
            auto deleter = [](void* ptr) {
                delete static_cast<TcpConnectionManager*>(ptr);
            };
            
            connection_manager_ = std::unique_ptr<void, std::function<void(void*)>>(
                connection_manager.release(), deleter);
            
            static_cast<TcpConnectionManager*>(connection_manager_.get())->Start();
            std::cout << "[ASIO] TCP连接管理器启动完成，使用端口: " << port << std::endl;
            return;
        } catch (const std::exception& e) {
            std::cout << "[ASIO] 端口 " << port << " 被占用，尝试端口 " << (port + 1) << std::endl;
            port++;
        }
    }
    
    std::cerr << "[ASIO] 无法找到可用的TCP连接端口" << std::endl;
}

void CommunicationManager::OnServiceDiscovered(const ServiceInfo& service) {
    std::lock_guard<std::mutex> lock(services_mutex_);
    discovered_services_[service.service_id] = service;
    std::cout << "[ASIO] 服务已添加到发现列表: " << service.service_id 
              << " (" << service.service_name << ") 地址: " << service.address 
              << ":" << service.port << std::endl;
    std::cout << "[ASIO] 当前发现的服务数量: " << discovered_services_.size() << std::endl;
}

void CommunicationManager::OnMessageReceived(const std::string& service_id, const std::vector<uint8_t>& data) {
    if (message_handler_) {
        message_handler_(service_id, data);
    }
}

void CommunicationManager::OnConnectionChanged(const std::string& service_id, bool connected) {
    if (connection_handler_) {
        connection_handler_(service_id, connected);
    }
}

void CommunicationManager::OnConnectionError(const std::string& service_id, const asio::error_code& ec) {
    if (error_handler_) {
        error_handler_(service_id, ec);
    }
}

void CommunicationManager::OnBroadcastMessageReceived(const std::vector<uint8_t>& data) {
    // 处理广播消息
    if (message_handler_) {
        message_handler_("broadcast", data);
    }
}

}  // namespace perception
