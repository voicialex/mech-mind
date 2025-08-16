#include "CommunicationInterface.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

namespace perception {

CommunicationInterface::CommunicationInterface(const Config &config) : config_(config) {
    // 创建通信管理器
    CommunicationManager::Config comm_config;
    comm_config.local_address = config.local_address;
    comm_config.local_port = config.local_port;
    comm_config.discovery_port = config.discovery_port;
    comm_config.max_connections = config.max_connections;
    comm_config.buffer_size = config.buffer_size;
    comm_config.heartbeat_interval = config.heartbeat_interval;
    comm_config.connection_timeout = config.connection_timeout;
    comm_config.enable_auto_reconnect = config.enable_auto_reconnect;
    comm_config.max_reconnect_attempts = config.max_reconnect_attempts;
    comm_config.reconnect_delay = config.reconnect_delay;
    comm_config.is_server = config.is_server; // 传递服务器模式标志
    
    comm_manager_ = std::make_unique<CommunicationManager>(comm_config);
}

CommunicationInterface::CommunicationInterface() : config_() {
    comm_manager_ = std::make_unique<CommunicationManager>();
}

CommunicationInterface::~CommunicationInterface() {
    Cleanup();
}

bool CommunicationInterface::Initialize() {
    if (initialized_) return true;
    
    try {
        // 注册回调函数
        comm_manager_->RegisterMessageHandler([this](const std::string& service_id, const std::vector<uint8_t>& data) {
            OnMessageReceived(service_id, data);
        });
        
        comm_manager_->RegisterConnectionHandler([this](const std::string& service_id, bool connected) {
            OnConnectionStatus(service_id, connected);
        });
        
        comm_manager_->RegisterErrorHandler([this](const std::string& service_id, const asio::error_code& ec) {
            OnError(service_id, ec.value());
        });
        
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "CommunicationInterface初始化失败: " << e.what() << std::endl;
        return false;
    }
}

bool CommunicationInterface::Start() {
    if (!initialized_) return false;
    if (running_) return true;
    
    try {
        // 启动通信管理器
        if (!comm_manager_->Start()) {
            return false;
        }
        
        // 注册本地服务
        CommunicationManager::ServiceInfo local_service;
        local_service.service_id = config_.local_service_id;
        local_service.service_name = config_.local_service_name;
        local_service.address = config_.local_address;
        local_service.port = config_.local_port;
        local_service.version = "1.0.0";
        local_service.capabilities = {"message", "discovery", "connection"};
        local_service.last_heartbeat = 0;
        local_service.is_online = true;
        
        if (!comm_manager_->RegisterService(local_service)) {
            return false;
        }
        
        running_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "CommunicationInterface启动失败: " << e.what() << std::endl;
        return false;
    }
}

void CommunicationInterface::Stop() {
    if (!running_) return;
    
    running_ = false;
    
    if (comm_manager_) {
        comm_manager_->UnregisterService();
        comm_manager_->Stop();
    }
}

void CommunicationInterface::Cleanup() {
    Stop();
    initialized_ = false;
}

void CommunicationInterface::RegisterMessageCallback(MessageCallback callback) {
    message_callback_ = std::move(callback);
}

void CommunicationInterface::RegisterConnectionCallback(ConnectionCallback callback) {
    connection_callback_ = std::move(callback);
}

void CommunicationInterface::RegisterErrorCallback(ErrorCallback callback) {
    error_callback_ = std::move(callback);
}

bool CommunicationInterface::ConnectToService(const std::string &service_id) {
    if (!running_) return false;
    
    return comm_manager_->ConnectToService(service_id);
}

void CommunicationInterface::DisconnectFromService(const std::string &service_id) {
    if (comm_manager_) {
        comm_manager_->DisconnectFromService(service_id);
    }
}

bool CommunicationInterface::SendMessage(const std::string &target_id, const Message::Ptr &message) {
    if (!running_ || !message) return false;
    
    try {
        // 序列化消息
        std::vector<uint8_t> data = message->Serialize();
        return comm_manager_->SendMessage(target_id, data);
    } catch (const std::exception& e) {
        std::cerr << "发送消息失败: " << e.what() << std::endl;
        return false;
    }
}

Message::Ptr CommunicationInterface::SendRequest(const std::string &target_id, const Message::Ptr &message, uint32_t timeout_ms) {
    if (!running_ || !message) return nullptr;
    
    try {
        // 序列化消息
        std::vector<uint8_t> data = message->Serialize();
        std::vector<uint8_t> response_data = comm_manager_->SendRequest(target_id, data, timeout_ms);
        
        if (response_data.empty()) {
            return nullptr;
        }
        
        // 反序列化响应
        // 简化实现：返回空消息
        return nullptr;
    } catch (const std::exception& e) {
        std::cerr << "发送请求失败: " << e.what() << std::endl;
        return nullptr;
    }
}

void CommunicationInterface::BroadcastMessage(const Message::Ptr &message, const std::string &service_name) {
    if (!running_ || !message) return;
    
    try {
        // 序列化消息
        std::vector<uint8_t> data = message->Serialize();
        comm_manager_->BroadcastMessage(data, service_name);
    } catch (const std::exception& e) {
        std::cerr << "广播消息失败: " << e.what() << std::endl;
    }
}

std::vector<CommunicationManager::ServiceInfo> CommunicationInterface::DiscoverServices(const std::string &service_name) {
    if (!running_) return {};
    
    return comm_manager_->DiscoverServices(service_name);
}

bool CommunicationInterface::IsServiceOnline(const std::string &service_id) const {
    if (!comm_manager_) return false;
    
    return comm_manager_->IsServiceOnline(service_id);
}

CommunicationManager::ConnectionInfo CommunicationInterface::GetConnectionInfo(const std::string &service_id) const {
    if (!comm_manager_) return {};
    
    return comm_manager_->GetConnectionInfo(service_id);
}

std::vector<CommunicationManager::ConnectionInfo> CommunicationInterface::GetAllConnections() const {
    if (!comm_manager_) return {};
    
    return comm_manager_->GetAllConnections();
}

CommunicationManager::ServiceInfo CommunicationInterface::GetLocalServiceInfo() const {
    if (!comm_manager_) return {};
    
    return comm_manager_->GetLocalServiceInfo();
}

std::string CommunicationInterface::GetStatistics() const {
    if (!comm_manager_) return "{}";
    
    return comm_manager_->GetStatistics();
}

bool CommunicationInterface::IsInitialized() const {
    return initialized_;
}

bool CommunicationInterface::IsRunning() const {
    return running_;
}

// 私有方法实现
void CommunicationInterface::OnMessageReceived(const std::string& service_id, const std::vector<uint8_t>& data) {
    try {
        // 反序列化消息
        // 简化实现：创建空消息
        auto message = std::make_shared<Message>();
        if (message && message_callback_) {
            message_callback_(message);
        }
    } catch (const std::exception& e) {
        std::cerr << "处理接收消息失败: " << e.what() << std::endl;
    }
}

void CommunicationInterface::OnConnectionStatus(const std::string& service_id, bool connected) {
    if (connection_callback_) {
        connection_callback_(service_id, connected);
    }
}

void CommunicationInterface::OnError(const std::string& service_id, uint16_t error_code) {
    if (error_callback_) {
        error_callback_(service_id, error_code);
    }
}

}  // namespace perception
