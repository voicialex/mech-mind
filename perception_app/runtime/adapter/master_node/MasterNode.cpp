#include "MasterNode.hpp"
#include "Logger.hpp"
#include "message/IMessageProtocol.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace perception;

MasterNode::MasterNode(const Config& config)
    : config_(config) {
}

MasterNode::~MasterNode() {
    Stop();
}

bool MasterNode::Initialize() {
    if (initialized_) {
        return true;
    }
    
    try {
        LOG_INFO_STREAM << "初始化Master节点 - 服务ID: " << config_.device_server_config.id;
        
        // 创建服务发现配置
        UdpServiceDiscoveryConfig discovery_config;
        discovery_config.local_address = config_.discovery_address;
        discovery_config.discovery_port = config_.discovery_port;
        discovery_config.broadcast_interval = config_.broadcast_interval;
        discovery_config.enable_broadcast = true;
        
        // 创建服务发现组件
        service_discovery_ = std::make_shared<UdpServiceDiscovery>(discovery_config);
        
        // 创建服务器端点（直接使用统一的EndpointConfig）
        server_ = std::make_shared<EndpointServer>(config_.device_server_config);
        
        // 注册事件处理器
        event_handler_ = std::make_shared<MasterNodeEventHandler>(this);
        server_->RegisterEventHandler(event_handler_);
        
        // 消息创建器注册由具体的应用层负责
        
        initialized_ = true;
        LOG_INFO_STREAM << "Master节点初始化完成";
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM << "Master节点初始化失败: " << e.what();
        return false;
    }
}

bool MasterNode::Start() {
    if (!initialized_) {
        LOG_ERROR_STREAM << "Master节点未初始化，无法启动";
        return false;
    }
    
    if (running_) {
        LOG_INFO_STREAM << "Master节点已在运行中";
        return true;
    }
    
    try {
        LOG_INFO_STREAM << "启动Master节点 - 服务ID: " << config_.device_server_config.id;
        
        // 启动服务器
        if (!server_->Initialize() || !server_->Start()) {
            LOG_ERROR_STREAM << "服务器启动失败";
            return false;
        }
        
        // 注册本地服务信息
        EndpointIdentity service_info;
        service_info.id = config_.device_server_config.id;
        service_info.name = config_.device_server_config.name;
        service_info.address = config_.device_server_config.address;
        service_info.port = config_.device_server_config.port;
        service_info.type = config_.device_server_config.type;
        
        // 注册服务
        service_discovery_->RegisterService(service_info);
        
        // 启动状态监控
        LOG_INFO_STREAM << "[MON] 启动Master节点状态监控线程";
        StartStatusMonitoring();
        
        running_ = true;
        LOG_INFO_STREAM << "Master节点启动完成";
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM << "Master节点启动失败: " << e.what();
        return false;
    }
}

void MasterNode::Stop() {
    if (!running_) {
        return;
    }
    
    LOG_INFO_STREAM << "停止Master节点 - 服务ID: " << config_.device_server_config.id;
    
    // 停止状态监控
    StopStatusMonitoring();
    
    // 停止服务发现
    if (service_discovery_) {
        service_discovery_->Stop();
    }
    
    // 停止服务器
    if (server_) {
        server_->Stop();
    }
    
    // 清空客户端列表
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_.clear();
    }
    
    running_ = false;
    LOG_INFO_STREAM << "Master节点已停止";
}

// 节点统计改由外部检查器输出

std::vector<ConnectionInfo> MasterNode::GetClientList() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    std::vector<ConnectionInfo> client_list;
    client_list.reserve(clients_.size());
    
    for (const auto& pair : clients_) {
        client_list.push_back(pair.second);
    }
    
    return client_list;
}

ConnectionInfo MasterNode::GetClientInfo(const std::string& endpoint_id) const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(endpoint_id);
    if (it != clients_.end()) {
        return it->second;
    }
    return ConnectionInfo();
}

size_t MasterNode::GetConnectedClientCount() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    size_t count = 0;
    for (const auto& pair : clients_) {
        if (ConnectionUtils::IsConnected(pair.second.state)) {
            count++;
        }
    }
    return count;
}

size_t MasterNode::GetOfflineClientCount() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    size_t count = 0;
    for (const auto& pair : clients_) {
        if (!ConnectionUtils::IsConnected(pair.second.state)) {
            count++;
        }
    }
    return count;
}

bool MasterNode::DisconnectClient(const std::string& endpoint_id) {
    if (!server_) {
        return false;
    }
    
    // 尝试断开连接
    bool success = server_->DisconnectClient(endpoint_id);
    if (success) {
        LOG_INFO_STREAM << "主动断开客户端连接: " << endpoint_id;
        
        // 更新本地状态
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(endpoint_id);
        if (it != clients_.end()) {
            it->second.state = ConnectionState::Disconnected;
        }
    } else {
        LOG_WARNING_STREAM << "断开客户端连接失败: " << endpoint_id;
    }
    
    return success;
}

size_t MasterNode::CleanupOfflineClients() {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    size_t cleaned_count = 0;
    std::vector<std::string> clients_to_remove;
    
    for (const auto& pair : clients_) {
        if (!ConnectionUtils::IsConnected(pair.second.state) && IsClientTimedOut(pair.second)) {
            clients_to_remove.push_back(pair.first);
        }
    }
    
    for (const auto& client_id : clients_to_remove) {
        clients_.erase(client_id);
        cleaned_count++;
        LOG_INFO_STREAM << "清理离线客户端: " << client_id;
    }
    
    return cleaned_count;
}

bool MasterNode::IsClientOnline(const std::string& endpoint_id) const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(endpoint_id);
    if (it != clients_.end()) {
        return ConnectionUtils::IsConnected(it->second.state) && !IsClientTimedOut(it->second);
    }
    return false;
}

uint64_t MasterNode::GetClientConnectionDuration(const std::string& endpoint_id) const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(endpoint_id);
    if (it != clients_.end()) {
        auto now = GetCurrentTimestamp();
        return (now - it->second.connect_time) / 1000; // 转换为秒
    }
    return 0;
}

uint64_t MasterNode::GetClientLastActivity(const std::string& endpoint_id) const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(endpoint_id);
    if (it != clients_.end()) {
        return it->second.remote_endpoint.last_activity;
    }
    return 0;
}



size_t MasterNode::SyncClientStates() {
    if (!server_) {
        return 0;
    }
    
    SyncClientStatesFromServer();
    
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_.size();
}

void MasterNode::OnServiceDiscovered(const EndpointIdentity& service_info) {
    LOG_INFO_STREAM << "发现客户端服务 - ID: " << service_info.id
                    << ", 名称: " << service_info.name
                    << ", 地址: " << service_info.address << ":" << service_info.port;
}

void MasterNode::AddOrUpdateClient(const std::string& endpoint_id, const ConnectionInfo& connection_info) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    auto it = clients_.find(endpoint_id);
    if (it != clients_.end()) {
        // 更新现有客户端
        auto& client = it->second;
        client.state = ConnectionState::Connected;
        client.remote_endpoint = connection_info.remote_endpoint;
        client.connect_time = GetCurrentTimestamp();
        client.reconnect_attempts = 0; // 重置重连次数
        
        LOG_INFO_STREAM << "更新客户端状态: " << endpoint_id;
    } else {
        // 添加新客户端
        clients_[endpoint_id] = connection_info;
        clients_[endpoint_id].connect_time = GetCurrentTimestamp();
        
        LOG_INFO_STREAM << "添加新客户端: " << endpoint_id 
                        << " (" << connection_info.remote_endpoint.address 
                        << ":" << connection_info.remote_endpoint.port << ")";
    }
}

void MasterNode::UpdateClientDisconnection(const std::string& endpoint_id) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    auto it = clients_.find(endpoint_id);
    if (it != clients_.end()) {
        it->second.state = ConnectionState::Disconnected;
        LOG_INFO_STREAM << "客户端断开连接: " << endpoint_id;
    }
}

void MasterNode::UpdateClientActivity(const std::string& endpoint_id) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    auto it = clients_.find(endpoint_id);
    if (it != clients_.end()) {
        auto now = GetCurrentTimestamp();
        it->second.remote_endpoint.last_activity = now;
        it->second.remote_endpoint.activity_count++;
    }
}

void MasterNode::UpdateClientError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    auto it = clients_.find(endpoint_id);
    if (it != clients_.end()) {
        it->second.state = ConnectionState::Error;
        auto now = GetCurrentTimestamp();
        it->second.remote_endpoint.last_activity = now;
        
        LOG_ERROR_STREAM << "客户端错误状态更新: " << endpoint_id 
                         << ", 错误码: " << error_code 
                         << ", 错误信息: " << error_message;
    }
}

void MasterNode::StartStatusMonitoring() {
    if (monitoring_running_.load()) {
        return;
    }
    
    monitoring_running_.store(true);
    LOG_INFO_STREAM << "[HB] Master节点监控线程启动中...";
    status_monitoring_thread_ = std::thread(&MasterNode::StatusMonitoringLoop, this);
    LOG_INFO_STREAM << "[HB] Master节点客户端状态监控线程已启动";
}

void MasterNode::StopStatusMonitoring() {
    if (!monitoring_running_.load()) {
        return;
    }
    
    monitoring_running_.store(false);
    
    if (status_monitoring_thread_.joinable()) {
        status_monitoring_thread_.join();
    }
    
    LOG_INFO_STREAM << "停止客户端状态监控线程";
}

void MasterNode::StatusMonitoringLoop() {
    uint64_t last_sync_time = 0;
    
    LOG_INFO_STREAM << "[MON] Master节点监控循环已启动，检查间隔=" << config_.status_check_interval << "ms";
    
    while (monitoring_running_.load()) {
        try {
            auto now = GetCurrentTimestamp();
            
            // 定期同步客户端状态
            if (now - last_sync_time >= config_.state_sync_interval) {
                SyncClientStatesFromServer();
                last_sync_time = now;
            }
            
            CheckClientTimeouts();
            
            // 如果启用自动清理，则清理超时的离线客户端
            if (config_.enable_auto_cleanup) {
                CleanupOfflineClients();
            }
            
            // 等待下一次检查
            std::this_thread::sleep_for(std::chrono::milliseconds(config_.status_check_interval));
        } catch (const std::exception& e) {
            LOG_ERROR_STREAM << "[MON] Master节点状态监控循环异常: " << e.what();
        }
    }
    LOG_INFO_STREAM << "[MON] Master节点监控循环已退出";
}

void MasterNode::CheckClientTimeouts() {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto now = GetCurrentTimestamp();
    
    for (auto& pair : clients_) {
        auto& client = pair.second;
        if (ConnectionUtils::IsConnected(client.state) && IsClientTimedOut(client)) {
            client.state = ConnectionState::Disconnected;
            LOG_WARNING_STREAM << "客户端超时断开: " << pair.first;
        }
    }
}



void MasterNode::SyncClientStatesFromServer() {
    if (!server_) {
        return;
    }
    
    // 获取服务器中的客户端状态
    auto server_clients = server_->GetAllClients();
    auto online_clients = server_->GetOnlineClients();
    
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    // 更新现有客户端状态
    for (auto& pair : clients_) {
        const std::string& client_id = pair.first;
        auto& client_info = pair.second;
        
        // 检查客户端是否在服务器中
        bool found_in_server = false;
        bool is_online_in_server = false;
        
        for (const auto& server_client : server_clients) {
            if (server_client.id == client_id) {
                found_in_server = true;
                UpdateClientStateFromServer(server_client);
                break;
            }
        }
        
        for (const auto& online_client : online_clients) {
            if (online_client.id == client_id) {
                is_online_in_server = true;
                break;
            }
        }
        
        // 如果客户端不在服务器中，标记为断开
        if (!found_in_server) {
            client_info.state = ConnectionState::Disconnected;
        } else if (is_online_in_server) {
            // 如果客户端在服务器中且在线，确保状态为连接
            if (!ConnectionUtils::IsConnected(client_info.state)) {
                client_info.state = ConnectionState::Connected;
            }
        } else {
            // 如果客户端在服务器中但不在线，标记为断开
            client_info.state = ConnectionState::Disconnected;
        }
    }
    
    // 添加服务器中有但本地没有的客户端
    for (const auto& server_client : server_clients) {
        auto it = clients_.find(server_client.id);
        if (it == clients_.end()) {
            // 创建新的客户端连接信息
            ConnectionInfo new_client;
            new_client.remote_endpoint = server_client;
            new_client.connect_time = GetCurrentTimestamp();
            new_client.state = server_client.is_active ? ConnectionState::Connected : ConnectionState::Disconnected;
            
            clients_[server_client.id] = new_client;
            LOG_INFO_STREAM << "同步添加客户端: " << server_client.id;
        }
    }
}



// 心跳识别已下沉到EndpointService层

uint64_t MasterNode::GetCurrentTimestamp() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

bool MasterNode::IsClientTimedOut(const ConnectionInfo& client_info) const {
    auto now = GetCurrentTimestamp();
    return (now - client_info.remote_endpoint.last_activity) > config_.client_timeout_interval;
}

void MasterNode::UpdateClientStateFromServer(const EndpointIdentity& server_client) {
    // 更新客户端状态信息
    auto it = clients_.find(server_client.id);
    if (it != clients_.end()) {
        auto& client = it->second;
        client.remote_endpoint.last_activity = server_client.last_activity;
        client.remote_endpoint.activity_count = server_client.activity_count;
        
        // 如果服务器显示客户端活跃，但本地状态不是连接，则更新状态
        if (server_client.is_active && !ConnectionUtils::IsConnected(client.state)) {
            client.state = ConnectionState::Connected;
        }
    }
}
