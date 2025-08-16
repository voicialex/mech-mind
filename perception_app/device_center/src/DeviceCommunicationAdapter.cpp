#include "DeviceCommunicationAdapter.hpp"
#include "communication/CommunicationInterface.hpp"
#include <iostream>
#include <sstream>
#include <random>
#include <nlohmann/json.hpp>

using namespace device_center;

namespace {
uint64_t now_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

std::string generate_message_id() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 999999);
    
    std::ostringstream oss;
    oss << "msg_" << now_ms() << "_" << dis(gen);
    return oss.str();
}
}  // namespace

DeviceCommunicationAdapter::DeviceCommunicationAdapter(const Config& config) 
    : config_(config), start_time_(0) {}

DeviceCommunicationAdapter::~DeviceCommunicationAdapter() {
    Cleanup();
}

bool DeviceCommunicationAdapter::Initialize() {
    if (initialized_) return true;
    
    try {
        // 创建底层通信接口
        perception::CommunicationInterface::Config comm_config;
        comm_config.local_service_id = config_.service_id;
        comm_config.local_service_name = config_.service_name;
        comm_config.local_address = config_.local_address;
        comm_config.local_port = config_.local_port;
        comm_config.discovery_port = config_.discovery_port;
        comm_config.max_connections = 50;
        comm_config.buffer_size = 8192;
        comm_config.heartbeat_interval = config_.heartbeat_interval;
        comm_config.connection_timeout = config_.connection_timeout;
        comm_config.enable_auto_reconnect = config_.enable_auto_reconnect;
        comm_config.max_reconnect_attempts = config_.max_reconnect_attempts;
        comm_config.reconnect_delay = 1000;
        comm_config.is_server = (config_.mode == Mode::Server);
        
        comm_interface_ = std::make_unique<perception::CommunicationInterface>(comm_config);
        
        // 注册底层回调
        comm_interface_->RegisterMessageCallback([this](const std::shared_ptr<perception::Message>& message) {
            OnUnderlyingMessageReceived(message);
        });
        
        comm_interface_->RegisterConnectionCallback([this](const std::string& service_id, bool connected) {
            OnUnderlyingConnectionChanged(service_id, connected);
        });
        
        comm_interface_->RegisterErrorCallback([this](const std::string& service_id, uint16_t error_code) {
            OnUnderlyingError(service_id, error_code);
        });
        
        initialized_ = true;
        start_time_ = now_ms();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "DeviceCommunicationAdapter初始化失败: " << e.what() << std::endl;
        return false;
    }
}

bool DeviceCommunicationAdapter::Start() {
    if (!initialized_) return false;
    if (running_) return true;
    
    try {
        if (comm_interface_->Start()) {
            running_ = true;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "DeviceCommunicationAdapter启动失败: " << e.what() << std::endl;
    }
    
    return false;
}

void DeviceCommunicationAdapter::Stop() {
    if (!running_) return;
    
    running_ = false;
    
    if (comm_interface_) {
        comm_interface_->Stop();
    }
}

void DeviceCommunicationAdapter::Cleanup() {
    Stop();
    initialized_ = false;
}

bool DeviceCommunicationAdapter::ConnectToService(const std::string& service_id) {
    if (!running_) return false;
    
    try {
        bool result = comm_interface_->ConnectToService(service_id);
        if (result) {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            connection_states_[service_id] = ConnectionState::Connected;
        }
        return result;
    } catch (const std::exception& e) {
        std::cerr << "连接服务失败: " << e.what() << std::endl;
        return false;
    }
}

void DeviceCommunicationAdapter::DisconnectFromService(const std::string& service_id) {
    if (comm_interface_) {
        comm_interface_->DisconnectFromService(service_id);
        
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connection_states_[service_id] = ConnectionState::Disconnected;
    }
}

bool DeviceCommunicationAdapter::SendMessage(const DeviceMessage& message) {
    if (!running_) return false;
    
    try {
        std::vector<uint8_t> data = SerializeMessage(message);
        
        if (message.target_id.empty()) {
            // 广播消息
            comm_interface_->BroadcastMessage(std::make_shared<perception::Message>());
        } else {
            // 点对点消息
            auto msg = std::make_shared<perception::Message>();
            if (comm_interface_->SendMessage(message.target_id, msg)) {
                messages_sent_++;
                return true;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "发送消息失败: " << e.what() << std::endl;
        connection_errors_++;
    }
    
    return false;
}

DeviceCommunicationAdapter::DeviceMessage DeviceCommunicationAdapter::SendRequest(const DeviceMessage& request, uint32_t timeout_ms) {
    if (!running_) return DeviceMessage{};
    
    try {
        std::vector<uint8_t> data = SerializeMessage(request);
        
        // 简化实现：直接返回空响应
        DeviceMessage response;
        response.type = MessageType::Error;
        response.source_id = config_.service_id;
        response.target_id = request.source_id;
        response.timestamp = now_ms();
        response.message_id = GenerateMessageId();
        
        return response;
    } catch (const std::exception& e) {
        std::cerr << "发送请求失败: " << e.what() << std::endl;
        connection_errors_++;
        return DeviceMessage{};
    }
}

void DeviceCommunicationAdapter::BroadcastMessage(const DeviceMessage& message, const std::string& service_name) {
    if (!running_) return;
    
    try {
        std::vector<uint8_t> data = SerializeMessage(message);
        comm_interface_->BroadcastMessage(std::make_shared<perception::Message>(), service_name);
        messages_sent_++;
    } catch (const std::exception& e) {
        std::cerr << "广播消息失败: " << e.what() << std::endl;
        connection_errors_++;
    }
}

std::vector<std::string> DeviceCommunicationAdapter::DiscoverServices(const std::string& service_name) {
    if (!running_) return {};
    
    try {
        auto services = comm_interface_->DiscoverServices(service_name);
        std::vector<std::string> result;
        for (const auto& service : services) {
            result.push_back(service.service_id);
        }
        return result;
    } catch (const std::exception& e) {
        std::cerr << "发现服务失败: " << e.what() << std::endl;
        return {};
    }
}

bool DeviceCommunicationAdapter::IsServiceOnline(const std::string& service_id) const {
    if (!comm_interface_) return false;
    
    try {
        return comm_interface_->IsServiceOnline(service_id);
    } catch (const std::exception& e) {
        return false;
    }
}

DeviceCommunicationAdapter::ConnectionState DeviceCommunicationAdapter::GetConnectionState(const std::string& service_id) const {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = connection_states_.find(service_id);
    if (it != connection_states_.end()) {
        return it->second;
    }
    return ConnectionState::Disconnected;
}

nlohmann::json DeviceCommunicationAdapter::GetStatistics() const {
    nlohmann::json stats;
    stats["initialized"] = initialized_.load();
    stats["running"] = running_.load();
    stats["start_time"] = start_time_;
    stats["uptime_ms"] = now_ms() - start_time_;
    stats["messages_sent"] = messages_sent_.load();
    stats["messages_received"] = messages_received_.load();
    stats["connection_errors"] = connection_errors_.load();
    
    std::lock_guard<std::mutex> lock(connections_mutex_);
    stats["connections"] = connection_states_.size();
    
    return stats;
}

bool DeviceCommunicationAdapter::IsInitialized() const {
    return initialized_;
}

bool DeviceCommunicationAdapter::IsRunning() const {
    return running_;
}

void DeviceCommunicationAdapter::RegisterMessageCallback(MessageCallback callback) {
    message_callback_ = std::move(callback);
}

void DeviceCommunicationAdapter::RegisterConnectionCallback(ConnectionCallback callback) {
    connection_callback_ = std::move(callback);
}

void DeviceCommunicationAdapter::RegisterErrorCallback(ErrorCallback callback) {
    error_callback_ = std::move(callback);
}

void DeviceCommunicationAdapter::OnUnderlyingMessageReceived(const std::shared_ptr<perception::Message>& message) {
    try {
        // 简化实现：创建空的设备消息
        DeviceMessage device_message;
        device_message.type = MessageType::Error;
        device_message.timestamp = now_ms();
        device_message.message_id = GenerateMessageId();
        
        messages_received_++;
        
        if (message_callback_) {
            message_callback_(device_message);
        }
    } catch (const std::exception& e) {
        std::cerr << "处理底层消息失败: " << e.what() << std::endl;
    }
}

void DeviceCommunicationAdapter::OnUnderlyingConnectionChanged(const std::string& service_id, bool connected) {
    ConnectionState state = connected ? ConnectionState::Connected : ConnectionState::Disconnected;
    
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connection_states_[service_id] = state;
    }
    
    if (connection_callback_) {
        connection_callback_(service_id, state);
    }
}

void DeviceCommunicationAdapter::OnUnderlyingError(const std::string& service_id, uint16_t error_code) {
    connection_errors_++;
    
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connection_states_[service_id] = ConnectionState::Error;
    }
    
    if (error_callback_) {
        error_callback_(service_id, error_code);
    }
}

std::vector<uint8_t> DeviceCommunicationAdapter::SerializeMessage(const DeviceMessage& message) {
    nlohmann::json j;
    j["type"] = static_cast<int>(message.type);
    j["source_id"] = message.source_id;
    j["target_id"] = message.target_id;
    j["timestamp"] = message.timestamp;
    j["data"] = message.data;
    j["message_id"] = message.message_id;
    
    std::string json_str = j.dump();
    return std::vector<uint8_t>(json_str.begin(), json_str.end());
}

DeviceCommunicationAdapter::DeviceMessage DeviceCommunicationAdapter::DeserializeMessage(const std::vector<uint8_t>& data) {
    DeviceMessage message;
    
    try {
        std::string json_str(data.begin(), data.end());
        nlohmann::json j = nlohmann::json::parse(json_str);
        
        message.type = static_cast<MessageType>(j["type"].get<int>());
        message.source_id = j["source_id"].get<std::string>();
        message.target_id = j["target_id"].get<std::string>();
        message.timestamp = j["timestamp"].get<uint64_t>();
        message.data = j["data"];
        message.message_id = j["message_id"].get<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "反序列化消息失败: " << e.what() << std::endl;
        message.type = MessageType::Error;
    }
    
    return message;
}

std::string DeviceCommunicationAdapter::GenerateMessageId() {
    return generate_message_id();
}

uint64_t DeviceCommunicationAdapter::GetCurrentTimestamp() {
    return now_ms();
}
