#include "DeviceServer.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using namespace device_center;

namespace {
uint64_t now_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
}  // namespace

DeviceServer::DeviceServer(const ServerConfig& config) 
    : DeviceManager(config), server_config_(config) {}

DeviceServer::DeviceServer() 
    : DeviceManager(), server_config_() {}

DeviceServer::~DeviceServer() {}

bool DeviceServer::Initialize(const std::string& config_path) {
    return DeviceManager::Initialize(config_path);
}

DeviceRegisterResponse DeviceServer::RegisterDevice(const DeviceRegisterRequest& request) {
    DeviceRegisterResponse resp{};
    
    if (!ValidateDeviceRegistration(request)) {
        resp.success = false;
        resp.message = "invalid registration";
        resp.error_code = 1;
  return resp;
}

    const std::string device_id = request.device_id.empty() ? 
        GenerateDeviceId(request) : request.device_id;

    // 创建设备信息
    DeviceInfo device_info;
    device_info.device_id = device_id;
    device_info.device_name = request.device_name;
    device_info.device_model = request.device_model;
    device_info.device_version = request.device_version;
    device_info.capabilities = request.capabilities;
    device_info.status = DeviceStatus::Online;
    device_info.status_message = "registered";
    device_info.last_heartbeat = now_ms();

    // 注册设备
    if (DeviceManager::RegisterDevice(device_id, device_info, request.config)) {
        resp.success = true;
        resp.device_id = device_id;
        resp.message = "device registered successfully";
        
        // 添加服务器配置
        resp.server_config["max_devices"] = server_config_.max_devices;
        resp.server_config["heartbeat_interval"] = server_config_.heartbeat_interval;
        resp.server_config["device_timeout"] = server_config_.device_timeout;
    } else {
        resp.success = false;
        resp.message = "device registration failed";
        resp.error_code = 2;
    }

    return resp;
  }

DeviceDiscoveryResponse DeviceServer::DiscoverDevices(const DeviceDiscoveryRequest& request) {
    DeviceDiscoveryResponse resp{};
    
    auto all_devices = GetAllDevices();
    
    for (const auto& device : all_devices) {
        if (request.device_name.empty() || device.device_name == request.device_name) {
            resp.devices.push_back(device);
        }
    }
    
    resp.success = true;
    resp.message = "discovery completed";
    return resp;
}

DeviceControlResponse DeviceServer::ControlDevice(const DeviceControlRequest& request) {
    DeviceControlResponse resp{};
    
    // 创建设备命令
    DeviceCommand command;
    command.device_id = request.device_id;
    command.command_type = request.command_type;
    command.parameters = request.parameters;
    command.timeout_ms = request.timeout_ms;
    command.require_response = request.require_response;
    command.timestamp = now_ms();
    
    // 通过事件处理器处理命令
    DeviceResponse response = SendDeviceCommand(command);
    
    resp.success = response.success;
    resp.message = response.message;
    resp.device_id = response.device_id;
    resp.command_type = command.command_type;
    resp.result = response.data;
    resp.error_code = response.error_code;
    
    return resp;
}

DeviceResponse DeviceServer::SendDeviceCommand(const DeviceCommand& command) {
    // 这里应该通过通信适配器发送命令到设备
    // 简化实现：直接返回成功响应
    
    DeviceResponse response;
    response.device_id = command.device_id;
    response.command_id = command.command_id;
    response.timestamp = now_ms();
    
    if (command.command_type == "status") {
        response.success = true;
        response.message = "Status query successful";
        response.data["status"] = "online";
        response.data["timestamp"] = response.timestamp;
    } else if (command.command_type == "control") {
        response.success = true;
        response.message = "Control command executed";
        response.data["result"] = "success";
    } else {
        response.success = false;
        response.message = "Unknown command type";
        response.error_code = 1001;
    }
    
    total_commands_processed_++;
    return response;
}

void DeviceServer::ReceiveDeviceData(const DeviceData& device_data) {
    // 通知事件处理器
    NotifyDeviceDataReceived(device_data);
}

void DeviceServer::BroadcastDeviceData(const DeviceData& device_data, 
                                     const std::vector<std::string>& target_clients) {
    // 这里可以实现向特定客户端广播数据的逻辑
    // 暂时只是接收数据
    ReceiveDeviceData(device_data);
}

nlohmann::json DeviceServer::GetServerStatistics() const {
    nlohmann::json stats = DeviceManager::GetStatistics();
    
    // 添加服务器特有的统计信息
    stats["total_connections"] = total_connections_.load();
    stats["total_commands_processed"] = total_commands_processed_.load();
    stats["max_devices"] = server_config_.max_devices;
    stats["max_clients"] = server_config_.max_clients;
    stats["enable_device_discovery"] = server_config_.enable_device_discovery;
    stats["enable_auto_approval"] = server_config_.enable_auto_approval;
    
    return stats;
}

void DeviceServer::HandleDeviceMessage(const DeviceCommunicationAdapter::DeviceMessage& message) {
    try {
        switch (message.type) {
            case DeviceCommunicationAdapter::MessageType::DeviceRegister: {
                // 处理设备注册消息
                if (message.data.contains("device_id")) {
                    std::string device_id = message.data["device_id"];
                    std::cout << "收到设备注册消息: " << device_id << std::endl;
                }
                break;
            }
            case DeviceCommunicationAdapter::MessageType::DeviceStatus: {
                // 处理设备状态消息
                if (message.data.contains("device_id")) {
                    std::string device_id = message.data["device_id"];
                    std::cout << "收到设备状态消息: " << device_id << std::endl;
                }
                break;
            }
            case DeviceCommunicationAdapter::MessageType::DeviceCommand: {
                // 处理设备命令消息
                if (message.data.contains("device_id")) {
                    std::string device_id = message.data["device_id"];
                    std::cout << "收到设备命令消息: " << device_id << std::endl;
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

void DeviceServer::HandleConnectionStateChanged(const std::string& service_id, 
                                             DeviceCommunicationAdapter::ConnectionState state) {
    std::cout << "连接状态变化: " << service_id << " -> " 
              << static_cast<int>(state) << std::endl;
    
    if (state == DeviceCommunicationAdapter::ConnectionState::Connected) {
        total_connections_++;
  }
}

void DeviceServer::HandleCommunicationError(const std::string& service_id, uint16_t error_code) {
  std::cerr << "通信错误: " << service_id << " 错误码: " << error_code << std::endl;
    NotifyDeviceError(service_id, error_code, "Communication error");
}

bool DeviceServer::ValidateDeviceRegistration(const DeviceRegisterRequest& request) {
    return !request.device_name.empty() && !request.device_model.empty();
}

std::string DeviceServer::GenerateDeviceId(const DeviceRegisterRequest& request) {
    return request.device_name + "_" + request.device_model + "_" + std::to_string(now_ms());
}
