#include "DeviceClient.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using namespace device_center;

namespace {
uint64_t now_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
}  // namespace

DeviceClient::DeviceClient(const ClientConfig& config) 
    : DeviceManager(config), client_config_(config) {}

DeviceClient::DeviceClient() 
    : DeviceManager(), client_config_() {}

DeviceClient::~DeviceClient() {}

bool DeviceClient::Initialize(const std::string& config_path) {
    return DeviceManager::Initialize(config_path);
}

bool DeviceClient::ConnectToServer(const std::string& server_id) {
    if (!IsInitialized()) return false;
    
    try {
        if (server_id.empty()) {
            // 自动发现服务器
            auto servers = DiscoverServers("Device Management Server");
            if (!servers.empty()) {
                return ConnectToServer(servers[0]);
            }
            return false;
        }
        
        // 连接到指定服务器
        if (GetCommunicationAdapter()->ConnectToService(server_id)) {
          connected_ = true;
            connected_server_id_ = server_id;
            std::cout << "成功连接到服务器: " << server_id << std::endl;
          return true;
    }
  } catch (const std::exception& e) {
    std::cerr << "连接服务器异常: " << e.what() << std::endl;
  }
  
  return false;
}

void DeviceClient::DisconnectFromServer() { 
    if (GetCommunicationAdapter()) {
        GetCommunicationAdapter()->DisconnectFromService(connected_server_id_);
  }
  connected_ = false; 
    connected_server_id_.clear();
}

bool DeviceClient::RegisterDevice(const std::string& device_id, const nlohmann::json& config) {
  // 先本地注册
    DeviceInfo device_info;
    device_info.device_id = device_id;
    device_info.device_name = device_id;
    device_info.device_model = "Client-Device";
    device_info.device_version = "1.0.0";
    device_info.capabilities = {DeviceCapability::Capture, DeviceCapability::Configure, DeviceCapability::Status};
    device_info.status = DeviceStatus::Online;
    device_info.status_message = "registered";
    device_info.last_heartbeat = now_ms();
    
    bool local_success = DeviceManager::RegisterDevice(device_id, device_info, config);

  // 如果已连接到服务器，向服务器注册设备
  if (connected_ && local_success) {
        // 通过通信适配器发送注册消息
        DeviceCommunicationAdapter::DeviceMessage message;
        message.type = DeviceCommunicationAdapter::MessageType::DeviceRegister;
        message.source_id = GetConfig().service_id;
        message.target_id = connected_server_id_;
        message.timestamp = now_ms();
        message.message_id = "reg_" + device_id;
        message.data["device_id"] = device_id;
        message.data["device_name"] = device_id;
        message.data["config"] = config;
        
        if (GetCommunicationAdapter()->SendMessage(message)) {
          std::cout << "向服务器注册设备: " << device_id << std::endl;
          return true;
    }
  }

  return local_success;
}

bool DeviceClient::RegisterDevice(const DeviceConfig& device_config) {
    nlohmann::json config = device_config.config;
    return RegisterDevice(device_config.device_id, config);
}

bool DeviceClient::ReportDeviceStatus(const std::string& device_id, const DeviceStatusInfo& status) {
    // 先本地更新状态
    bool local_success = UpdateDeviceStatus(device_id, status);
    total_status_reports_++;
    
    // 如果已连接服务器，上报状态
    if (connected_ && local_success) {
        DeviceCommunicationAdapter::DeviceMessage message;
        message.type = DeviceCommunicationAdapter::MessageType::DeviceStatus;
        message.source_id = GetConfig().service_id;
        message.target_id = connected_server_id_;
        message.timestamp = now_ms();
        message.message_id = "status_" + device_id;
        message.data["device_id"] = device_id;
        message.data["status"] = static_cast<int>(status.status);
        message.data["status_message"] = status.status_message;
        message.data["timestamp"] = status.timestamp;
        
        GetCommunicationAdapter()->SendMessage(message);
    }
    
    return local_success;
}

bool DeviceClient::ReportDeviceData(const DeviceData& device_data) {
    total_data_reports_++;
    
    // 如果已连接服务器，上报数据
    if (connected_) {
        DeviceCommunicationAdapter::DeviceMessage message;
        message.type = DeviceCommunicationAdapter::MessageType::DeviceData;
        message.source_id = GetConfig().service_id;
        message.target_id = connected_server_id_;
        message.timestamp = now_ms();
        message.message_id = "data_" + device_data.device_id;
        message.data["device_id"] = device_data.device_id;
        message.data["data_type"] = device_data.data_type;
        message.data["data"] = device_data.data;
        message.data["data_size"] = device_data.data_size;
        
        GetCommunicationAdapter()->SendMessage(message);
  }
  
  return true;
}

bool DeviceClient::RespondToCommand(const std::string& device_id, 
                                 const std::string& command_id, 
                                 const DeviceResponse& response) {
    total_commands_processed_++;
    
    // 如果已连接服务器，发送响应
    if (connected_) {
        DeviceCommunicationAdapter::DeviceMessage message;
        message.type = DeviceCommunicationAdapter::MessageType::DeviceResponse;
        message.source_id = GetConfig().service_id;
        message.target_id = connected_server_id_;
        message.timestamp = now_ms();
        message.message_id = "resp_" + command_id;
        message.data["device_id"] = device_id;
        message.data["command_id"] = command_id;
        message.data["success"] = response.success;
        message.data["message"] = response.message;
        message.data["data"] = response.data;
        message.data["error_code"] = response.error_code;
        
        GetCommunicationAdapter()->SendMessage(message);
    }
    
  return true;
}

std::vector<std::string> DeviceClient::DiscoverServers(const std::string& server_name) {
    if (GetCommunicationAdapter()) {
        return GetCommunicationAdapter()->DiscoverServices(server_name);
  }
  return {};
}

bool DeviceClient::IsConnectedToServer() const {
    return connected_;
}

nlohmann::json DeviceClient::GetConnectionStatus() const {
  nlohmann::json status_json;
  status_json["connected"] = connected_.load();
  status_json["server_id"] = connected_server_id_;
    
    if (GetCommunicationAdapter()) {
        status_json["adapter_stats"] = GetCommunicationAdapter()->GetStatistics();
    }
    
  return status_json;
}

nlohmann::json DeviceClient::GetClientStatistics() const {
    nlohmann::json stats_json = DeviceManager::GetStatistics();
    
    // 添加客户端特有的统计信息
    stats_json["total_status_reports"] = total_status_reports_.load();
    stats_json["total_data_reports"] = total_data_reports_.load();
    stats_json["total_commands_processed"] = total_commands_processed_.load();
    stats_json["connected"] = connected_.load();
    stats_json["server_id"] = connected_server_id_;
    
  return stats_json;
}

bool DeviceClient::SetDeviceConfig(const std::string& device_id, const nlohmann::json& config) {
    std::lock_guard<std::mutex> lk(device_configs_mutex_);
  auto it = device_configs_.find(device_id);
  if (it == device_configs_.end()) return false;
  it->second.config = config;
  return true;
}

nlohmann::json DeviceClient::GetDeviceConfig(const std::string& device_id) const {
    std::lock_guard<std::mutex> lk(device_configs_mutex_);
  auto it = device_configs_.find(device_id);
  if (it != device_configs_.end()) return it->second.config;
  return nlohmann::json::object();
}

bool DeviceClient::EnableDeviceHeartbeat(const std::string& device_id, uint32_t interval_ms) {
    std::lock_guard<std::mutex> lk(device_configs_mutex_);
  device_heartbeat_enabled_[device_id] = true;
  return true;
}

bool DeviceClient::DisableDeviceHeartbeat(const std::string& device_id) {
    std::lock_guard<std::mutex> lk(device_configs_mutex_);
  device_heartbeat_enabled_[device_id] = false;
  return true;
}

void DeviceClient::HandleDeviceMessage(const DeviceCommunicationAdapter::DeviceMessage& message) {
    try {
        switch (message.type) {
            case DeviceCommunicationAdapter::MessageType::DeviceCommand: {
                // 处理来自服务器的设备命令
                if (message.data.contains("device_id") && message.data.contains("command_type")) {
                    std::string device_id = message.data["device_id"];
                    std::string command_type = message.data["command_type"];
                    
                    DeviceCommand command;
                    command.device_id = device_id;
                    command.command_type = command_type;
                    command.parameters = message.data.value("parameters", nlohmann::json::object());
                    command.timestamp = message.timestamp;
                    command.command_id = message.message_id;
                    
                    HandleServerCommand(command);
                }
                break;
            }
            case DeviceCommunicationAdapter::MessageType::DeviceStatus: {
                // 处理来自服务器的状态请求
                if (message.data.contains("device_id")) {
                    std::string device_id = message.data["device_id"];
                    HandleServerStatusRequest(device_id);
                }
                break;
            }
            case DeviceCommunicationAdapter::MessageType::DeviceData: {
                // 处理来自服务器的数据请求
                if (message.data.contains("device_id")) {
                    std::string device_id = message.data["device_id"];
                    HandleServerDataRequest(device_id);
                }
                break;
            }
            default:
                std::cout << "收到未知类型的设备消息" << std::endl;
                break;
        }
    } catch (const std::exception& e) {
        std::cerr << "处理设备消息异常: " << e.what() << std::endl;
    }
}

void DeviceClient::HandleConnectionStateChanged(const std::string& service_id, 
                                             DeviceCommunicationAdapter::ConnectionState state) {
    std::cout << "连接状态变化: " << service_id << " -> " 
              << static_cast<int>(state) << std::endl;
    
    if (state == DeviceCommunicationAdapter::ConnectionState::Connected) {
        connected_ = true;
        connected_server_id_ = service_id;
    } else if (state == DeviceCommunicationAdapter::ConnectionState::Disconnected) {
        connected_ = false;
        connected_server_id_.clear();
    }
}

void DeviceClient::HandleCommunicationError(const std::string& service_id, uint16_t error_code) {
    std::cerr << "通信错误: " << service_id << " 错误码: " << error_code << std::endl;
    NotifyDeviceError(service_id, error_code, "Communication error");
}

void DeviceClient::HandleServerCommand(const DeviceCommand& command) {
    std::cout << "收到服务器命令: " << command.device_id << " - " << command.command_type << std::endl;
    
    // 这里应该根据命令类型执行相应的操作
    // 简化实现：直接返回成功响应
    
    DeviceResponse response;
    response.device_id = command.device_id;
    response.command_id = command.command_id;
    response.success = true;
    response.message = "Command executed successfully";
    response.timestamp = now_ms();
    response.data["result"] = "success";
    
    RespondToCommand(command.device_id, command.command_id, response);
    total_commands_processed_++;
}

void DeviceClient::HandleServerStatusRequest(const std::string& device_id) {
    std::cout << "收到服务器状态请求: " << device_id << std::endl;
    
    // 获取设备状态并上报
    DeviceStatusInfo status = GetDeviceStatus(device_id);
    ReportDeviceStatus(device_id, status);
}

void DeviceClient::HandleServerDataRequest(const std::string& device_id) {
    std::cout << "收到服务器数据请求: " << device_id << std::endl;
    
    // 这里应该获取设备数据并上报
    // 简化实现：创建空的设备数据
    DeviceData data;
    data.device_id = device_id;
    data.data_type = "status";
    data.timestamp = now_ms();
    data.data = nlohmann::json::object();
    data.data_size = 0;
    
    ReportDeviceData(data);
}

void DeviceClient::AutoReconnect() {
    // 简化实现：省略自动重连逻辑
}

void DeviceClient::SendHeartbeat() {
    // 简化实现：省略心跳发送逻辑
}

bool DeviceClient::ValidateDeviceRegistration(const std::string& device_id) {
    return !device_id.empty();
}

void DeviceClient::UpdateConnectionStatus(bool connected, const std::string& server_id) {
    connected_ = connected;
  if (connected) {
        connected_server_id_ = server_id;
  } else {
    connected_server_id_.clear();
  }
}
