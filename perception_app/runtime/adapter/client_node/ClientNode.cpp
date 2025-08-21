#include "ClientNode.hpp"
#include "Logger.hpp"
#include "message/IMessageProtocol.hpp"
#include <iostream>
#include <sstream>
#include <functional>

using namespace perception;

ClientNode::ClientNode(const Config& config)
    : config_(config) {
}

ClientNode::~ClientNode() {
    Stop();
}

bool ClientNode::Initialize() {
    if (initialized_) {
        return true;
    }
    
    try {
        LOG_INFO_STREAM << "初始化Client节点";
        
        // 服务发现配置
        UdpServiceDiscoveryConfig discovery_config;
        discovery_config.local_address = config_.discovery_address;
        discovery_config.discovery_port = config_.discovery_port;
        discovery_config.enable_broadcast = false; // 客户端不需要广播
        discovery_config.broadcast_interval = 0;
        service_discovery_ = std::make_shared<UdpServiceDiscovery>(discovery_config);
        
        // 设备客户端配置
        EndpointConfig device_config;
        device_config.id = config_.device_client_id;
        device_config.name = config_.device_client_name;
        device_config.type = EndpointType::Client;
        device_config.address = "0.0.0.0";
        device_config.port = 0;
        device_config.enable_auto_reconnect = true;
        device_config.max_reconnect_attempts = 10;
        device_config.reconnect_interval = 5000;
        device_config.connection_check_interval = 10000;
        device_config.heartbeat_interval = 30000;
        device_client_ = std::make_shared<EndpointClient>(device_config);
        
        // 控制器客户端配置
        if (config_.enable_controller_client) {
            EndpointConfig controller_config;
            controller_config.id = config_.controller_client_id;
            controller_config.name = config_.controller_client_name;
            controller_config.type = EndpointType::Client;
            controller_config.address = "0.0.0.0";
            controller_config.port = 0;
            controller_config.enable_auto_reconnect = true;
            controller_config.max_reconnect_attempts = 10;
            controller_config.reconnect_interval = 5000;
            controller_config.connection_check_interval = 10000;
            controller_config.heartbeat_interval = 30000;
            controller_client_ = std::make_shared<EndpointClient>(controller_config);
        }
        
        // 注册不同事件处理器
        device_event_handler_ = std::make_shared<DeviceClientEventHandler>(this);
        device_client_->RegisterEventHandler(device_event_handler_);
        
        if (controller_client_) {
            controller_event_handler_ = std::make_shared<ControllerClientEventHandler>(this);
            controller_client_->RegisterEventHandler(controller_event_handler_);
        }
        
        // 消息创建器注册由具体的应用层负责
        
        initialized_ = true;
        LOG_INFO_STREAM << "Client节点初始化完成";
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM << "Client节点初始化失败: " << e.what();
        return false;
    }
}

bool ClientNode::Start() {
    if (!initialized_) {
        LOG_ERROR_STREAM << "Client节点未初始化，无法启动";
        return false;
    }
    if (running_) {
        LOG_INFO_STREAM << "Client节点已在运行中";
        return true;
    }
    try {
        LOG_INFO_STREAM << "启动Client节点";
        using std::placeholders::_1;
        service_discovery_->DiscoverServices("", std::bind(&ClientNode::OnServiceDiscovered, this, _1));
        
        if (!device_client_->Initialize() || !device_client_->Start()) {
            LOG_ERROR_STREAM << "设备客户端启动失败";
            service_discovery_->Stop();
            return false;
        }
        
        if (controller_client_) {
            if (!controller_client_->Initialize() || !controller_client_->Start()) {
                LOG_ERROR_STREAM << "控制器客户端启动失败";
                device_client_->Stop();
                service_discovery_->Stop();
                return false;
            }
        }
        
        running_ = true;
        LOG_INFO_STREAM << "Client节点启动完成";
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM << "Client节点启动失败: " << e.what();
        return false;
    }
}

void ClientNode::Stop() {
    if (!running_) {
        return;
    }
    
    LOG_INFO_STREAM << "停止Client节点";
    
    // 停止控制器客户端
    if (controller_client_) {
        controller_client_->Stop();
    }
    
    // 停止中心设备客户端
    if (device_client_) {
        device_client_->Stop();
    }
    
    // 停止服务发现
    if (service_discovery_) {
        service_discovery_->Stop();
    }
    
    running_ = false;
    LOG_INFO_STREAM << "Client节点已停止";
}

// 节点统计改由外部检查器输出

void ClientNode::OnServiceDiscovered(const EndpointIdentity& service_info) {
    LOG_INFO_STREAM << "发现服务 - ID: " << service_info.id
                    << ", 名称: " << service_info.name
                    << ", 地址: " << service_info.address << ":" << service_info.port
                    << ", 类型: " << static_cast<int>(service_info.type);

    // 根据服务类型和名称进行过滤
    if (service_info.type == EndpointType::Server) {
        // 连接到设备服务器（原 center_device_server）
        if (service_info.id == "device_server" ||
            service_info.name.find("Device Server") != std::string::npos ||
            service_info.name.find("Center Device Server") != std::string::npos) {
            LOG_INFO_STREAM << "匹配到设备服务，尝试连接: "
                            << service_info.address << ":" << service_info.port;
            if (device_client_) {
                device_client_->ConnectToServer(service_info.address, service_info.port);
            }
        }
        // 控制器服务器
        if (controller_client_ && 
            (service_info.id == "controller_server" ||
             service_info.name.find("Controller Server") != std::string::npos)) {
            LOG_INFO_STREAM << "匹配到控制器服务，尝试连接: "
                            << service_info.address << ":" << service_info.port;
            controller_client_->ConnectToServer(service_info.address, service_info.port);
        }
    }
}

// 设备客户端事件处理器实现
void ClientNode::DeviceClientEventHandler::OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) {
    if (!node_) return;
    LOG_INFO_STREAM << "[Device] 收到消息 - 端点: " << endpoint_id 
                    << ", 数据大小: " << message_data.size() << " 字节";
}
void ClientNode::DeviceClientEventHandler::OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) {
    if (!node_) return;
    if (connected) {
        LOG_INFO_STREAM << "[Device] 连接成功 - 端点: " << endpoint_id
                        << ", 地址: " << connection_info.remote_endpoint.address
                        << ":" << connection_info.remote_endpoint.port;
    } else {
        LOG_INFO_STREAM << "[Device] 连接断开 - 端点: " << endpoint_id
                        << ", 地址: " << connection_info.remote_endpoint.address
                        << ":" << connection_info.remote_endpoint.port;
    }
}
void ClientNode::DeviceClientEventHandler::OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) {
    if (!node_) return;
    LOG_ERROR_STREAM << "[Device] 错误 - 端点: " << endpoint_id 
                     << ", 错误码: " << error_code 
                     << ", 错误信息: " << error_message;
}

// 控制器客户端事件处理器实现
void ClientNode::ControllerClientEventHandler::OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) {
    if (!node_) return;
    LOG_INFO_STREAM << "[Controller] 收到消息 - 端点: " << endpoint_id 
                    << ", 数据大小: " << message_data.size() << " 字节";
}
void ClientNode::ControllerClientEventHandler::OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) {
    if (!node_) return;
    if (connected) {
        LOG_INFO_STREAM << "[Controller] 连接成功 - 端点: " << endpoint_id
                        << ", 地址: " << connection_info.remote_endpoint.address
                        << ":" << connection_info.remote_endpoint.port;
    } else {
        LOG_INFO_STREAM << "[Controller] 连接断开 - 端点: " << endpoint_id
                        << ", 地址: " << connection_info.remote_endpoint.address
                        << ":" << connection_info.remote_endpoint.port;
    }
}
void ClientNode::ControllerClientEventHandler::OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) {
    if (!node_) return;
    LOG_ERROR_STREAM << "[Controller] 错误 - 端点: " << endpoint_id 
                     << ", 错误码: " << error_code 
                     << ", 错误信息: " << error_message;
}
