#include "EndpointServer.hpp"
#include "Logger.hpp"
#include "message/IMessageProtocol.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

using namespace perception;

EndpointServer::EndpointServer(const EndpointConfig& config)
    : EndpointService(config) {
}

EndpointServer::~EndpointServer() {
    Stop();
    Cleanup();
}

bool EndpointServer::Initialize() {
    if (!EndpointService::Initialize()) {
        return false;
    }
    
    // 启用心跳支持
    EnableHeartbeat(true);
    
    return true;
}

bool EndpointServer::Start() {
    if (!EndpointService::Start()) {
        return false;
    }
    
    LOG_INFO_STREAM << "[HB] 启动服务器心跳监控线程";
    StartMonitorThread();
    return true;
}

void EndpointServer::Stop() {
    std::cout << "正在停止服务器..." << std::endl;
    StopMonitorThread();
    EndpointService::Stop();
    std::cout << "服务器已停止" << std::endl;
}

void EndpointServer::Cleanup() {
    EndpointService::Cleanup();
    
    // 清理所有客户端
    std::lock_guard<std::mutex> lock(clients_mutex_);
    clients_.clear();
}

bool EndpointServer::RegisterClient(const std::string& client_id, const EndpointIdentity& client_info) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    if (clients_.size() >= config_.max_clients) {
        return false;
    }
    
    clients_[client_id] = client_info;
    statistics_.total_clients_registered++;
    
    return true;
}

bool EndpointServer::UnregisterClient(const std::string& client_id) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        clients_.erase(it);
        return true;
    }
    
    return false;
}

bool EndpointServer::DisconnectClient(const std::string& client_id) {
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(client_id);
        if (it != clients_.end()) {
            it->second.is_active = false;
        }
    }
    // 物理断开
    if (GetTransport()) {
        GetTransport()->Disconnect(client_id);
    }
    return true;
}

EndpointIdentity EndpointServer::GetClientInfo(const std::string& client_id) const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    auto it = clients_.find(client_id);
    return it != clients_.end() ? it->second : EndpointIdentity{};
}

std::vector<EndpointIdentity> EndpointServer::GetAllClients() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    std::vector<EndpointIdentity> result;
    for (const auto& [id, info] : clients_) {
        result.push_back(info);
    }
    
    return result;
}

std::vector<EndpointIdentity> EndpointServer::GetOnlineClients() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    std::vector<EndpointIdentity> result;
    for (const auto& [id, info] : clients_) {
        if (info.is_active) {
            result.push_back(info);
        }
    }
    
    return result;
}

bool EndpointServer::HasClient(const std::string& client_id) const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_.find(client_id) != clients_.end();
}

bool EndpointServer::IsClientOnline(const std::string& client_id) const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    
    auto it = clients_.find(client_id);
    return it != clients_.end() && it->second.is_active;
}

bool EndpointServer::SendToClient(const std::string& client_id, const std::vector<uint8_t>& message_data) {
    LOG_DEBUG_STREAM << "[TX] 服务器发送 -> client_id=" << client_id
                     << ", size=" << message_data.size() << " bytes";
    if (EndpointService::SendMessage(client_id, message_data)) {
        LOG_DEBUG_STREAM << "[TX] 服务器发送成功 -> client_id=" << client_id;
        return true;
    }
    LOG_WARNING_STREAM << "[TX] 服务器发送失败 -> client_id=" << client_id;
    
    return false;
}

std::vector<uint8_t> EndpointServer::SendRequestToClient(const std::string& client_id, 
                                                       const std::vector<uint8_t>& request_data, 
                                                       uint32_t timeout_ms) {
    // 简化实现：直接发送消息，不等待响应
    if (SendToClient(client_id, request_data)) {
        return {}; // 返回空响应
    }
    return {};
}

void EndpointServer::BroadcastToClients(const std::vector<uint8_t>& message_data, 
                                       const std::vector<std::string>& client_ids) {
    if (client_ids.empty()) {
        // 广播给所有在线客户端
        auto online_clients = GetOnlineClients();
        for (const auto& client : online_clients) {
            EndpointService::SendMessage(client.id, message_data);
        }
    } else {
        // 广播给指定客户端
        for (const auto& client_id : client_ids) {
            if (IsClientOnline(client_id)) {
                EndpointService::SendMessage(client_id, message_data);
            }
        }
    }
    
    statistics_.total_broadcasts++;
}

void EndpointServer::RegisterEventHandler(EventHandler::Ptr handler) {
    user_handler_ = handler;
    // 用内部处理器封装用户处理器，实现服务端内部状态维护
    EndpointService::RegisterEventHandler(std::make_shared<InternalServerEventHandler>(this));
}

bool EndpointServer::SendMessage(const std::string& target_id, const std::vector<uint8_t>& message_data, uint32_t timeout_ms) {
    if (timeout_ms > 0) {
        return SendRequestToClient(target_id, message_data, timeout_ms).size() > 0;
    } else {
        return SendToClient(target_id, message_data);
    }
}

void EndpointServer::BroadcastMessage(const std::vector<uint8_t>& message_data, 
                                    const std::string& target_name) {
    BroadcastToClients(message_data, {});
}

// 私有方法实现

void EndpointServer::CleanupOfflineClients() {
    uint64_t current_time = GetCurrentTimestamp();
    std::vector<std::string> offline_clients;
    
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (const auto& [id, info] : clients_) {
            if (!info.is_active && 
                (current_time - info.last_activity) > config_.client_timeout) {
                offline_clients.push_back(id);
            }
        }
    }
    
    for (const auto& client_id : offline_clients) {
        UnregisterClient(client_id);
    }
}

void EndpointServer::ProcessClientHeartbeats() {
    uint64_t current_time = GetCurrentTimestamp();
    
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto& [id, info] : clients_) {
        if (info.is_active && 
            (current_time - info.last_activity) > config_.client_timeout) {
            info.is_active = false;
            if (statistics_.total_clients_connected > 0) statistics_.total_clients_connected--;
        }
    }
}

uint64_t EndpointServer::GetCurrentTimestamp() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

bool EndpointServer::IsHeartbeatMessage(const std::vector<uint8_t>& message_data) const {
    // 使用基类的心跳检测方法
    return EndpointService::IsHeartbeatMessage(message_data);
}

void EndpointServer::StartMonitorThread() {
    if (monitor_running_.load()) return;
    monitor_running_.store(true);
    LOG_INFO_STREAM << "[HB] 服务器监控线程启动中...";
    monitor_thread_ = std::thread([this]() {
        const uint32_t interval = std::max(1000u, config_.client_timeout / 3);
        LOG_INFO_STREAM << "[HB] 服务器监控线程已启动，检查间隔=" << interval << "ms";
        while (monitor_running_.load()) {
            try {
                ProcessClientHeartbeats();
                CleanupOfflineClients();
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            } catch (...) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
        LOG_INFO_STREAM << "[HB] 服务器监控线程已退出";
    });
}

void EndpointServer::StopMonitorThread() {
    if (!monitor_running_.load()) return;
    monitor_running_.store(false);
    if (monitor_thread_.joinable()) monitor_thread_.join();
}

// 内部事件处理器：先维护，再转发
void EndpointServer::InternalServerEventHandler::OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) {
    {
        std::lock_guard<std::mutex> lock(server_->clients_mutex_);
        auto it = server_->clients_.find(endpoint_id);
        if (it != server_->clients_.end()) {
            it->second.last_activity = server_->GetCurrentTimestamp();
            it->second.is_active = true;
        }
    }
    // 打印心跳或普通消息的日志
    if (server_->IsHeartbeatMessage(message_data)) {
        LOG_DEBUG_STREAM << "[HB] 服务器收到心跳 <- client_id=" << endpoint_id
                         << ", size=" << message_data.size();
    } else {
        LOG_DEBUG_STREAM << "[RX] 服务器收到消息 <- client_id=" << endpoint_id
                         << ", size=" << message_data.size();
    }
    // 如果是心跳，可选择额外统计，这里直接转发
    if (server_->user_handler_) server_->user_handler_->OnMessageReceived(endpoint_id, message_data);
}

void EndpointServer::InternalServerEventHandler::OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) {
    bool previous_active = false;
    {
        std::lock_guard<std::mutex> lock(server_->clients_mutex_);
        auto& info = server_->clients_[endpoint_id];
        previous_active = info.is_active;
        info = connection_info.remote_endpoint; // 覆盖为远端身份
        info.is_active = connected;
        info.last_activity = server_->GetCurrentTimestamp();
        if (connected && !previous_active) {
            server_->statistics_.total_clients_connected++;
        } else if (!connected && previous_active) {
            if (server_->statistics_.total_clients_connected > 0) server_->statistics_.total_clients_connected--;
        }
    }
    if (server_->user_handler_) server_->user_handler_->OnConnectionChanged(endpoint_id, connected, connection_info);
}

void EndpointServer::InternalServerEventHandler::OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) {
    {
        std::lock_guard<std::mutex> lock(server_->clients_mutex_);
        auto it = server_->clients_.find(endpoint_id);
        if (it != server_->clients_.end()) {
            it->second.last_activity = server_->GetCurrentTimestamp();
        }
    }
    if (server_->user_handler_) server_->user_handler_->OnError(endpoint_id, error_code, error_message);
}
