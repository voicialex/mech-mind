#include "AsioTransport.hpp"
#include "Logger.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using namespace perception;

AsioTransport::AsioTransport(const EndpointConfig& config) 
    : config_(config) {}

AsioTransport::~AsioTransport() {
    Stop();
}

bool AsioTransport::Initialize() {
    try {
        LOG_INFO_STREAM << "[NET][INIT] AsioTransport初始化开始 - 模式: " << (config_.type == EndpointType::Server ? "服务器" : "客户端");
        
        // 创建IO上下文
        io_context_ = std::make_shared<asio::io_context>();
        work_guard_ = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(io_context_->get_executor());
        
        // 创建TCP接收器（如果是服务器模式）
        if (config_.type == EndpointType::Server) {
            LOG_INFO_STREAM << "[NET][ACCEPT][BIND] 创建TCP接收器 - 绑定地址: " << config_.address << ":" << config_.port;
            
            // 创建acceptor
            acceptor_ = std::make_unique<asio::ip::tcp::acceptor>(*io_context_);
            
            // 打开acceptor
            acceptor_->open(asio::ip::tcp::v4());
            
            // 设置端口复用选项
            acceptor_->set_option(asio::socket_base::reuse_address(true));
            // 注意：reuse_port在某些系统上可能不被支持，如果出现错误可以注释掉这行
            // acceptor_->set_option(asio::socket_base::reuse_port(true));

            // 绑定到指定地址和端口
            acceptor_->bind(asio::ip::tcp::endpoint(
                asio::ip::make_address(config_.address), 
                config_.port
            ));
            
            // 开始监听
            acceptor_->listen();
            
            LOG_INFO_STREAM << "[NET][ACCEPT][LISTEN] TCP接收器创建成功 - 监听端口: " << config_.port;
        } else {
            LOG_INFO_STREAM << "[NET][DIAL][LOCAL] 客户端模式 - 本地地址: " << config_.address << ":" << config_.port;
        }
        
        start_time_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        LOG_INFO_STREAM << "[NET][INIT] AsioTransport初始化完成";
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM << "[NET][INIT][ERR] AsioTransport初始化失败: " << e.what();
        return false;
    }
}

bool AsioTransport::Start() {
    if (running_) return true;
    
    try {
        LOG_INFO_STREAM << "[NET][START] AsioTransport启动开始";
        
        // 启动IO上下文线程
        StartIoContext();
        
        // 如果是服务器模式，开始接受连接
        if (config_.type == EndpointType::Server && acceptor_) {
            LOG_INFO_STREAM << "[NET][ACCEPT][START] 开始接受TCP连接 - 监听端口: " << config_.port;
            StartAccept();
        }
        
        running_ = true;
        LOG_INFO_STREAM << "[NET][START] AsioTransport启动完成";
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM << "[NET][START][ERR] AsioTransport启动失败: " << e.what();
        return false;
    }
}

void AsioTransport::Stop() {
    if (!running_) return;
    
    LOG_INFO_STREAM << "[NET][STOP] 正在停止AsioTransport";
    running_ = false;
    
    // 关闭所有连接
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        for (auto& [id, connection] : connections_) {
            connection->Close();
        }
        connections_.clear();
    }
    
    // 关闭接收器
    if (acceptor_) {
        acceptor_->close();
    }
    
    // 停止IO上下文
    StopIoContext();
    
    LOG_INFO_STREAM << "[NET][STOP] AsioTransport已停止";
}

bool AsioTransport::Connect(const std::string& service_id, const std::string& address, uint16_t port) {
    if (!running_) return false;
    
    try {
        LOG_INFO_STREAM << "[NET][DIAL] 尝试连接到服务 - ID: " << service_id << ", 地址: " << address << ":" << port;
        
        // 创建socket
        auto socket = std::make_shared<asio::ip::tcp::socket>(*io_context_);
        
        // 解析地址
        asio::ip::tcp::resolver resolver(*io_context_);
        auto endpoints = resolver.resolve(address, std::to_string(port));
        
        // 异步连接
        asio::async_connect(*socket, endpoints,
            [this, service_id, socket, address, port](const asio::error_code& ec, const asio::ip::tcp::endpoint&) {
                if (!ec) {
                    LOG_INFO_STREAM << "[NET][DIAL] 连接成功 - 服务ID: " << service_id << ", 远程地址: " << address << ":" << port;
                    // 连接成功，创建连接对象
                    auto connection = std::make_shared<TcpConnection>(std::move(*socket), service_id, this);
                    AddConnection(service_id, connection);
                    connection->Start();
                    
                    if (event_handler_) {
                        // 创建连接信息
                        ConnectionInfo connection_info;
                        connection_info.remote_endpoint.id = service_id;
                        connection_info.remote_endpoint.address = address;
                        connection_info.remote_endpoint.port = port;
                        connection_info.state = ConnectionState::Connected;
                        connection_info.remote_endpoint.is_active = true;
                        connection_info.connect_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count();
                        connection_info.remote_endpoint.last_activity = connection_info.connect_time;
                        
                        event_handler_->OnConnectionChanged(service_id, true, connection_info);
                    }
                } else {
                    LOG_ERROR_STREAM << "[NET][DIAL][ERR] 连接失败 - 服务ID: " << service_id << ", 地址: " << address << ":" << port << ", 错误: " << ec.message();
                    if (event_handler_) {
                        event_handler_->OnError(service_id, ec.value(), ec.message());
                    }
                }
            }
        );
        
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM << "[NET][DIAL][ERR] 连接异常 - 服务ID: " << service_id << ", 地址: " << address << ":" << port << ", 错误: " << e.what();
        return false;
    }
}

void AsioTransport::Disconnect(const std::string& service_id) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = connections_.find(service_id);
    if (it != connections_.end()) {
        it->second->Close();
        connections_.erase(it);
        
        if (event_handler_) {
            // 创建连接信息
            ConnectionInfo connection_info;
            connection_info.remote_endpoint.id = service_id;
            connection_info.state = ConnectionState::Disconnected;
            connection_info.remote_endpoint.is_active = false;
            connection_info.remote_endpoint.last_activity = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            
            event_handler_->OnConnectionChanged(service_id, false, connection_info);
        }
    }
}

bool AsioTransport::SendMessage(const std::string& target_id, const std::vector<uint8_t>& data) {
    if (!running_) return false;
    
    try {
        auto connection = GetConnection(target_id);
        if (connection) {
            bool result = connection->SendMessage(data);
            if (result) {
                messages_sent_++;
            }
            return result;
        }
        
        return false;
    } catch (const std::exception& e) {
        std::cerr << "[NET][TX][ERR] 发送消息失败: " << e.what() << std::endl;
        connection_errors_++;
        return false;
    }
}

bool AsioTransport::BroadcastMessage(const std::vector<uint8_t>& data, const std::string& target_filter) {
    if (!running_) return false;
    
    try {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        bool success = false;
        
        for (auto& [id, connection] : connections_) {
            if (target_filter.empty() || id.find(target_filter) != std::string::npos) {
                if (connection->SendMessage(data)) {
                    success = true;
                    messages_sent_++;
                }
            }
        }
        
        return success;
    } catch (const std::exception& e) {
        std::cerr << "[NET][TX][ERR] 广播消息失败: " << e.what() << std::endl;
        connection_errors_++;
        return false;
    }
}

void AsioTransport::RegisterEventHandler(EventHandler::Ptr handler) {
    event_handler_ = std::move(handler);
}

ConnectionInfo AsioTransport::GetConnectionInfo(const std::string& service_id) const {
    auto connection = GetConnection(service_id);
    if (connection) {
        return connection->GetConnectionInfo();
    }
    return ConnectionInfo{};
}

std::vector<ConnectionInfo> AsioTransport::GetAllConnections() const {
    std::vector<ConnectionInfo> result;
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    for (const auto& [id, connection] : connections_) {
        result.push_back(connection->GetConnectionInfo());
    }
    
    return result;
}

bool AsioTransport::IsConnected(const std::string& service_id) const {
    auto connection = GetConnection(service_id);
    return connection && connection->IsConnected();
}

// 统计接口由 TransportInspector 访问实现

bool AsioTransport::IsRunning() const {
    return running_;
}

// TcpConnection实现
AsioTransport::TcpConnection::TcpConnection(asio::ip::tcp::socket socket, const std::string& service_id, AsioTransport* owner)
    : socket_(std::move(socket)), service_id_(service_id), owner_(owner) {
    
    connection_info_.remote_endpoint.id = service_id;
    connection_info_.state = ConnectionState::Connected;
    connection_info_.remote_endpoint.is_active = true;
    connection_info_.remote_endpoint.last_activity = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    connection_info_.connect_time = connection_info_.remote_endpoint.last_activity;
    connection_info_.remote_endpoint.activity_count = 0;
    
    LOG_INFO_STREAM << "[NET][CONN] TCP连接创建 - 服务ID: " << service_id;
    
    // 在Start()方法中获取远程端点信息，而不是在构造函数中
}

void AsioTransport::TcpConnection::SetServiceId(const std::string& service_id) {
    service_id_ = service_id;
    connection_info_.remote_endpoint.id = service_id;
}

void AsioTransport::TcpConnection::Start() {
    // 获取远程端点信息
    if (socket_.is_open()) {
        try {
            auto endpoint = socket_.remote_endpoint();
            connection_info_.remote_endpoint.address = endpoint.address().to_string();
            connection_info_.remote_endpoint.port = endpoint.port();
            LOG_INFO_STREAM << "[NET][CONN] TCP连接启动 - 服务ID: " << service_id_ 
                            << ", 远程地址: " << connection_info_.remote_endpoint.address << ":" << connection_info_.remote_endpoint.port;
        } catch (const std::exception& e) {
            connection_info_.remote_endpoint.address = "unknown";
            connection_info_.remote_endpoint.port = 0;
            LOG_WARNING_STREAM << "[NET][CONN][WARN] 无法获取TCP连接远程端点信息 - 服务ID: " << service_id_ << ", 错误: " << e.what();
        }
    }
    
    StartRead();
}

void AsioTransport::TcpConnection::Close() {
    if (socket_.is_open()) {
        asio::error_code ec;
        socket_.close(ec);
    }
    connection_info_.state = ConnectionState::Disconnected;
    connection_info_.remote_endpoint.is_active = false;
    connection_info_.remote_endpoint.last_activity = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

bool AsioTransport::TcpConnection::SendMessage(const std::vector<uint8_t>& data) {
    if (!socket_.is_open()) return false;
    
    try {
        std::lock_guard<std::mutex> lock(write_mutex_);
        
        // 简单的消息格式：长度 + 数据
        uint32_t length = static_cast<uint32_t>(data.size());
        write_buffer_.clear();
        write_buffer_.resize(sizeof(length) + data.size());
        memcpy(write_buffer_.data(), &length, sizeof(length));
        memcpy(write_buffer_.data() + sizeof(length), data.data(), data.size());
        
        LOG_INFO_STREAM << "[NET][TX] 发送TCP消息 -> service_id=" << service_id_ 
                        << ", size=" << data.size() << " bytes";
        
        asio::async_write(socket_, asio::buffer(write_buffer_),
            [this](const asio::error_code& ec, std::size_t) {
                if (ec) {
                    // 处理写入错误
                    connection_info_.state = ConnectionState::Error;
                    LOG_ERROR_STREAM << "[NET][TX][ERR] TCP消息发送失败 - service_id=" << service_id_ << ", error=" << ec.message();
                } else {
                    connection_info_.remote_endpoint.activity_count++;
                    connection_info_.remote_endpoint.last_activity = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
                    LOG_INFO_STREAM << "[NET][TX] TCP消息发送成功 -> service_id=" << service_id_ 
                                   << ", count=" << connection_info_.remote_endpoint.activity_count;
                }
            }
        );
        
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM << "TCP消息发送异常 - 服务ID: " << service_id_ << ", 错误: " << e.what();
        return false;
    }
}

bool AsioTransport::TcpConnection::IsConnected() const {
    return socket_.is_open() && connection_info_.state == ConnectionState::Connected;
}

ConnectionInfo AsioTransport::TcpConnection::GetConnectionInfo() const {
    return connection_info_;
}

ConnectionInfo& AsioTransport::TcpConnection::GetConnectionInfoRef() {
    return connection_info_;
}

void AsioTransport::TcpConnection::StartRead() {
    // 先读取消息长度
    asio::async_read(socket_, asio::buffer(&message_length_, sizeof(message_length_)),
        [this](const asio::error_code& ec, std::size_t) {
            if (!ec) {
                message_data_.resize(message_length_);
                // 读取消息数据
                asio::async_read(socket_, asio::buffer(message_data_),
                    [this](const asio::error_code& ec, std::size_t) {
                        if (!ec) {
                            connection_info_.remote_endpoint.last_activity = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch()).count();
                            LOG_DEBUG_STREAM << "[NET][RX] 收到TCP消息 <- service_id=" << service_id_ 
                                             << ", size=" << message_data_.size();
                            // 将原始帧上抛给上层
                            if (owner_ && owner_->event_handler_) {
                                owner_->event_handler_->OnMessageReceived(service_id_, message_data_);
                            }
                            // 继续读取下一帧
                            StartRead();
                        } else {
                            // 处理读取错误
                            connection_info_.state = ConnectionState::Error;
                        }
                    }
                );
            } else {
                // 处理读取错误
                connection_info_.state = ConnectionState::Error;
            }
        }
    );
}

void AsioTransport::StartAccept() {
    if (!acceptor_) return;
    
    auto connection = std::make_shared<TcpConnection>(
        asio::ip::tcp::socket(*io_context_), "", this);
    
    LOG_INFO_STREAM << "[NET][ACCEPT][WAIT] 等待TCP连接...";
    
    acceptor_->async_accept(
        connection->GetSocket(),
        [this, connection](const asio::error_code& ec) {
            if (!ec) {
                // 生成连接ID
                std::string connection_id = "accepted_" + std::to_string(connection_counter_++);
                // 更新连接对象的service_id，确保后续回调携带正确的ID
                connection->SetServiceId(connection_id);
                
                // 更新连接信息
                try {
                    auto endpoint = connection->GetSocket().remote_endpoint();
                    auto& conn_info = connection->GetConnectionInfoRef();
                    conn_info.remote_endpoint.address = endpoint.address().to_string();
                    conn_info.remote_endpoint.port = endpoint.port();
                    conn_info.remote_endpoint.id = connection_id;
                    
                    LOG_INFO_STREAM << "[NET][ACCEPT] 接受TCP连接 - 连接ID: " << connection_id 
                                   << ", 远程地址: " << conn_info.remote_endpoint.address << ":" << conn_info.remote_endpoint.port;
                } catch (const std::exception& e) {
                    auto& conn_info = connection->GetConnectionInfoRef();
                    conn_info.remote_endpoint.address = "unknown";
                    conn_info.remote_endpoint.port = 0;
                    LOG_WARNING_STREAM << "[NET][ACCEPT][WARN] 无法获取接受连接的远程端点信息 - 连接ID: " << connection_id << ", 错误: " << e.what();
                }
                
                AddConnection(connection_id, connection);
                connection->Start();
                
                if (event_handler_) {
                    // 创建连接信息
                    ConnectionInfo connection_info;
                    connection_info.remote_endpoint.id = connection_id;
                    connection_info.remote_endpoint.address = connection->GetConnectionInfo().remote_endpoint.address;
                    connection_info.remote_endpoint.port = connection->GetConnectionInfo().remote_endpoint.port;
                    connection_info.state = ConnectionState::Connected;
                    connection_info.remote_endpoint.is_active = true;
                    connection_info.connect_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
                    connection_info.remote_endpoint.last_activity = connection_info.connect_time;
                    
                    event_handler_->OnConnectionChanged(connection_id, true, connection_info);
                }
            } else {
                LOG_ERROR_STREAM << "[NET][ACCEPT][ERR] 接受TCP连接失败: " << ec.message();
            }
            
            // 继续接受下一个连接
            StartAccept();
        }
    );
}

void AsioTransport::StartIoContext() {
    io_thread_ = std::thread([this]() {
        try {
            io_context_->run();
        } catch (const std::exception& e) {
            std::cerr << "IO上下文运行异常: " << e.what() << std::endl;
        }
    });
}

void AsioTransport::StopIoContext() {
    LOG_INFO_STREAM << "正在停止IO上下文";
    
    if (work_guard_) {
        work_guard_.reset();
    }
    
    if (io_context_) {
        io_context_->stop();
    }
    
    if (io_thread_.joinable()) {
        // 使用超时机制等待线程退出（缩短为1秒）
        auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(1);
        while (io_thread_.joinable() && 
               std::chrono::system_clock::now() < timeout) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        if (io_thread_.joinable()) {
            LOG_WARNING_STREAM << "强制分离IO线程";
            io_thread_.detach();
        } else {
            io_thread_.join();
        }
    }
    
    LOG_INFO_STREAM << "IO上下文已停止";
}

void AsioTransport::AddConnection(const std::string& service_id, std::shared_ptr<TcpConnection> connection) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_[service_id] = std::move(connection);
}

void AsioTransport::RemoveConnection(const std::string& service_id) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.erase(service_id);
}

std::shared_ptr<AsioTransport::TcpConnection> AsioTransport::GetConnection(const std::string& service_id) const {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = connections_.find(service_id);
    return it != connections_.end() ? it->second : nullptr;
}

// 静态成员初始化
uint32_t AsioTransport::connection_counter_ = 0;
