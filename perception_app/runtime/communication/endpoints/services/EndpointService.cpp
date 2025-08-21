#include "EndpointService.hpp"
#include "communication/transports/AsioTransport.hpp"
#include "message/IMessageProtocol.hpp"
#include "Logger.hpp"
#include <iostream>
#include <sstream>

using namespace perception;

EndpointService::EndpointService(const EndpointConfig& config)
    : config_(config), state_(EndpointState::Stopped), initialized_(false) {
    LOG_INFO_STREAM << "EndpointService 创建 - ID: " << config.id;
}

EndpointService::~EndpointService() {
    Cleanup();
}

bool EndpointService::Initialize() {
    if (initialized_.load()) {
        return true;
    }
    
    if (state_ != EndpointState::Stopped) {
        return false;
    }
    
    SetState(EndpointState::Starting);
    
    try {     
        // 初始化各个组件
        InitializeTransport();
        InitializeMessageProtocol();
        InitializeMessageRouter();
        
        statistics_.start_time = GetCurrentTimestamp();
        initialized_ = true;
        SetState(EndpointState::Stopped); // 初始化完成，回到停止状态
        return true;
    } catch (const std::exception& e) {
        std::cerr << "端点服务初始化失败: " << e.what() << std::endl;
        SetState(EndpointState::Error);
        return false;
    }
}

bool EndpointService::Start() {
    if (!initialized_.load()) {
        LOG_ERROR_STREAM << "端点服务未初始化，无法启动";
        return false;
    }
    
    if (state_ != EndpointState::Stopped) {
        LOG_WARNING_STREAM << "端点服务状态不正确，当前状态: " << static_cast<uint32_t>(state_.load());
        return false;
    }
    
    if (running_.load()) {
        LOG_INFO_STREAM << "端点服务已在运行中";
        return true;
    }
    
    LOG_INFO_STREAM << "启动端点服务 - 服务ID: " << config_.id 
                    << ", 类型: " << (config_.type == EndpointType::Server ? "服务器" : "客户端");
    
    SetState(EndpointState::Starting);
    
    try {
        // 初始化传输层
        if (transport_ && !transport_->Initialize()) {
            LOG_ERROR_STREAM << "传输层初始化失败";
            SetState(EndpointState::Error);
            return false;
        }
        
        // 启动传输层
        if (transport_ && !transport_->Start()) {
            LOG_ERROR_STREAM << "传输层启动失败";
            SetState(EndpointState::Error);
            return false;
        }
        
        running_ = true;
        SetState(EndpointState::Running);
        LOG_INFO_STREAM << "端点服务启动成功 - 服务ID: " << config_.id;
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM << "端点服务启动失败: " << e.what();
        SetState(EndpointState::Error);
        return false;
    }
}

void EndpointService::Stop() {
    if (!running_.load()) {
        return;
    }
    
    LOG_INFO_STREAM << "正在停止端点服务 - 服务ID: " << config_.id;
    SetState(EndpointState::Stopping);
    running_ = false;
    
    // 停止传输层
    if (transport_) {
        LOG_INFO_STREAM << "正在停止传输层";
        transport_->Stop();
    }
    
    SetState(EndpointState::Stopped);
    LOG_INFO_STREAM << "端点服务已停止 - 服务ID: " << config_.id;
}

void EndpointService::Cleanup() {
    Stop();
    
    // 清理连接
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        endpoint_connections_.clear();
    }
    
    // 清理事件处理器
    event_handler_.reset();
    
    // 重置组件
    transport_.reset();
    message_router_.reset();
    
    // 重置统计信息
    statistics_.Reset();
    initialized_ = false;
}

bool EndpointService::SendMessage(const std::string& target_id, const std::vector<uint8_t>& message_data, uint32_t timeout_ms) {
    if (!transport_ || !running_.load()) {
        return false;
    }
    
    if (ProcessOutgoingMessage(target_id, message_data)) {
        if (transport_->SendMessage(target_id, message_data)) {
            statistics_.messages_sent++;
            return true;
        }
    }
    
    statistics_.errors++;
        return false;
}

void EndpointService::BroadcastMessage(const std::vector<uint8_t>& message_data, 
                                     const std::string& target_name) {
    if (!transport_ || !running_.load()) {
        return;
    }
    
    transport_->BroadcastMessage(message_data, target_name);
    statistics_.messages_sent++;
}

void EndpointService::RegisterEventHandler(EventHandler::Ptr handler) {
    event_handler_ = std::move(handler);
}

bool EndpointService::IsEndpointOnline(const std::string& endpoint_id) const {
    return IsConnected(endpoint_id);
}

IEndpointService::EndpointState EndpointService::GetState() const {
    return state_.load();
}

const EndpointConfig& EndpointService::GetConfig() const {
    return config_;
}

bool EndpointService::IsRunning() const {
    return running_.load();
}

// 受保护方法实现
void EndpointService::SetState(EndpointState state) {
    state_.store(state);
}

std::string EndpointService::GenerateEndpointId() const {
    return config_.id + "_" + std::to_string(GetCurrentTimestamp());
}

uint64_t EndpointService::GetCurrentTimestamp() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

bool EndpointService::IsConnected(const std::string& endpoint_id) const {
    if (!transport_) {
        return false;
    }
    
    return transport_->IsConnected(endpoint_id);
}

std::vector<std::string> EndpointService::GetConnectedEndpoints() const {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    std::vector<std::string> connected_endpoints;
    
    for (const auto& [id, connected] : endpoint_connections_) {
        if (connected) {
            connected_endpoints.push_back(id);
        }
    }
    
    return connected_endpoints;
}

bool EndpointService::ProcessIncomingMessage(const std::string& endpoint_id, const std::vector<uint8_t>& data) {
    try {
        LOG_DEBUG_STREAM << "[RX] 收到消息 -> endpoint_id=" << endpoint_id << ", size=" << data.size() << " bytes";
        
        // 基础消息验证
        if (data.empty()) {
            LOG_WARNING_STREAM << "[RX] 收到空消息";
            return false;
        }
        
        // 验证消息格式（如果启用了消息协议）
        if (message_router_) {
            if (!MessageFactory::ValidateMessage(data)) {
                LOG_WARNING_STREAM << "[RX] 消息格式验证失败";
                return false;
            }
            
            // 创建消息对象
            auto message = MessageFactory::CreateFromBytes(data);
            if (!message) {
                LOG_WARNING_STREAM << "[RX] 创建消息对象失败";
                return false;
            }
            
            LOG_DEBUG_STREAM << "[RX] 消息解析成功 -> type=" << static_cast<int>(message->GetType()) 
                             << ", id=0x" << std::hex << message->GetMessageId() << std::dec;
            
            // 路由消息
            if (message_router_->HasHandler(message->GetMessageId())) {
                auto response = message_router_->RouteMessage(message);
                if (response) {
                    // 发送响应
                    auto response_data = response->Serialize();
                    SendMessage(endpoint_id, response_data);
                }
            }
        }
        
        statistics_.messages_received++;
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM << "[RX] 处理传入消息失败: " << e.what();
        return false;
    }
}
    
bool EndpointService::ProcessOutgoingMessage(const std::string& target_id, const std::vector<uint8_t>& data) {
    try {
        LOG_DEBUG_STREAM << "[TX] 发送消息 -> target_id=" << target_id << ", size=" << data.size() << " bytes";
        
        // 这里可以添加消息验证和处理逻辑
        // 使用新的消息系统，不再需要协议层检查
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_STREAM << "[TX] 处理传出消息失败: " << e.what();
        return false;
    }
}

// 私有方法实现
bool EndpointService::InitializeTransport() {
    LOG_INFO_STREAM << "初始化传输层 - 服务ID: " << config_.id 
                    << ", 类型: " << (config_.type == EndpointType::Server ? "服务器" : "客户端");
    
    // 直接使用EndpointConfig作为传输层配置
    transport_ = std::make_shared<AsioTransport>(config_);
    
    // 创建并注册内部事件处理器
    internal_event_handler_ = std::make_shared<InternalEventHandler>(this);
    transport_->RegisterEventHandler(internal_event_handler_);
    
    LOG_INFO_STREAM << "传输层初始化完成";
    return true;
}

void EndpointService::InitializeMessageProtocol() {
    // 使用新的消息协议系统
    // 初始化消息工厂
    MessageFactory::InitializeMessageTypeNames();
    MessageFactory::InitializeSubMessageTypeNames();
    MessageFactory::InitializeErrorCodeDescriptions();
    
    // 消息创建器注册由具体的应用层负责
    // 这里只初始化基础的消息协议框架
    
    LOG_DEBUG_STREAM << "消息协议初始化完成";
}

void EndpointService::InitializeMessageRouter() {
    // 使用新的消息路由器
    message_router_ = CreateMessageRouter();
    LOG_DEBUG_STREAM << "消息路由器初始化完成";
}

void EndpointService::OnMessageReceived(const std::string& service_id, const std::vector<uint8_t>& data) {
    if (ProcessIncomingMessage(service_id, data)) {
        if (event_handler_) {
            event_handler_->OnMessageReceived(service_id, data);
        }
    }
}

void EndpointService::OnConnectionChanged(const std::string& service_id, bool connected, const ConnectionInfo& connection_info) {
    LOG_INFO_STREAM << "[CONN] EndpointService::OnConnectionChanged 被调用 -> service_id=" << service_id 
                     << ", connected=" << connected;
    
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        endpoint_connections_[service_id] = connected;
    }
    
    if (event_handler_) {
        LOG_INFO_STREAM << "[CONN] 转发连接事件给事件处理器 -> service_id=" << service_id;
        event_handler_->OnConnectionChanged(service_id, connected, connection_info);
    } else {
        LOG_WARNING_STREAM << "[CONN] 事件处理器为空，无法转发连接事件 -> service_id=" << service_id;
    }
}

void EndpointService::OnError(const std::string& service_id, uint16_t error_code, const std::string& error_message) {
    statistics_.errors++;
    
    if (event_handler_) {
        event_handler_->OnError(service_id, error_code, error_message);
    }
}

bool EndpointService::IsHeartbeatMessage(const std::vector<uint8_t>& message_data) const {
    // 检查是否是协议帧格式的心跳消息
    if (message_data.size() < ProtocolFrame::HEADER_SIZE + 17) return false; // 最小帧长度12 + 心跳负载17
    
    // 检查魔数 (2字节)
    uint16_t magic_id = static_cast<uint16_t>(message_data[0]) | 
                       (static_cast<uint16_t>(message_data[1]) << 8);
    
    if (magic_id != ProtocolFrame::MAGIC_ID) return false;
    
    // 检查消息类型和ID (跳过CRC，从第4字节开始)
    if (message_data[4] != 0xFF || message_data[5] != 0xFF || message_data[6] != 0xFF) return false;
    
    // 检查负载中的心跳关键字
    const char key[] = {'H','E','A','R','T','B','E','A','T'};
    for (size_t i = 0; i < 9; ++i) {
        if (message_data[ProtocolFrame::HEADER_SIZE + i] != (uint8_t)key[i]) return false;
    }
    
    return true;
}
