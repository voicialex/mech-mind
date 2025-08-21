#include "master_node/MasterNode.hpp"
#include "Logger.hpp"
#include "configure/ConfigHelper.hpp"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace perception;

std::unique_ptr<MasterNode> g_master_node = nullptr;
std::atomic<bool> g_running{true};

void SignalHandler(int signal) {
    std::cout << "\n收到信号 " << signal << "，正在停止Master节点..." << std::endl;
    g_running = false;
    if (g_master_node) {
        g_master_node->Stop();
    }
    exit(0);
}

// 服务端事件处理器
class ServerEventHandler : public ITransport::EventHandler {
public:
    void OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) override {
        LOG_INFO_STREAM << "[MSG] 收到客户端消息 - 端点: " << endpoint_id 
                        << ", 数据大小: " << message_data.size() << " 字节";
        
        // 如果endpoint_id为空，尝试从服务器获取当前连接的客户端
        std::string actual_endpoint_id = endpoint_id;
        if (actual_endpoint_id.empty() && g_master_node && g_master_node->GetServer()) {
            auto online_clients = g_master_node->GetServer()->GetOnlineClients();
            if (!online_clients.empty()) {
                actual_endpoint_id = online_clients[0].id; // 使用第一个在线客户端
                LOG_INFO_STREAM << "[MSG] 使用默认客户端ID: " << actual_endpoint_id;
            }
        }
        
        // 解析客户端消息（使用消息协议格式）
        if (message_data.size() > 0) {
            try {
                // 尝试解析为协议消息
                auto message = MessageFactory::CreateFromBytes(message_data);
                if (message) {
                    // 成功解析协议消息
                    LOG_INFO_STREAM << "[MSG] 解析协议消息成功 - 类型: " << static_cast<int>(message->GetType())
                                   << ", ID: 0x" << std::hex << message->GetMessageId() << std::dec;
                    
                    // 获取消息负载
                    const auto& frame = message->GetFrame();
                    if (!frame.payload.empty()) {
                        std::string payload_str(frame.payload.begin(), frame.payload.end());
                        LOG_INFO_STREAM << "[MSG] 消息负载内容: " << payload_str;
                        
                        // 处理端口信息消息
                        if (payload_str.find("CLIENT_PORT_INFO:") == 0) {
                            std::string client_port = payload_str.substr(17); // 跳过 "CLIENT_PORT_INFO:"
                            LOG_INFO_STREAM << "[MSG] 客户端端口信息: " << client_port;
                            
                            // 构建回复消息
                            auto reply_message = MessageFactory::CreateMessage(
                                MessageType::Response,
                                MessageIds::DEVICE_STATUS,
                                SubMessageIds::READY
                            );
                            
                            std::string reply_payload = "SERVER_REPLY:9090_ACK_" + client_port;
                            std::vector<uint8_t> reply_data(reply_payload.begin(), reply_payload.end());
                            
                            // 设置回复消息的负载
                            ProtocolFrame reply_frame = reply_message->GetFrame();
                            reply_frame.payload = reply_data;
                            reply_frame.length = reply_data.size();
                            reply_message->SetFrame(reply_frame);
                            
                            // 序列化并发送回复
                            std::vector<uint8_t> serialized_reply = reply_message->Serialize();
                            
                            if (g_master_node && g_master_node->GetServer() && !actual_endpoint_id.empty()) {
                                if (g_master_node->GetServer()->SendToClient(actual_endpoint_id, serialized_reply)) {
                                    LOG_INFO_STREAM << "[MSG] 发送协议回复成功: " << reply_payload << " -> " << actual_endpoint_id;
                                } else {
                                    LOG_WARNING_STREAM << "[MSG] 发送协议回复失败 -> " << actual_endpoint_id;
                                }
                            } else {
                                LOG_WARNING_STREAM << "[MSG] 无法发送回复，客户端ID无效: " << actual_endpoint_id;
                            }
                        } else {
                            // 其他消息的通用回复
                            auto reply_message = MessageFactory::CreateMessage(
                                MessageType::Response,
                                MessageIds::DEVICE_STATUS,
                                SubMessageIds::READY
                            );
                            
                            std::string reply_payload = "SERVER_REPLY:UNKNOWN_MESSAGE";
                            std::vector<uint8_t> reply_data(reply_payload.begin(), reply_payload.end());
                            
                            ProtocolFrame reply_frame = reply_message->GetFrame();
                            reply_frame.payload = reply_data;
                            reply_frame.length = reply_data.size();
                            reply_message->SetFrame(reply_frame);
                            
                            std::vector<uint8_t> serialized_reply = reply_message->Serialize();
                            
                            if (g_master_node && g_master_node->GetServer() && !actual_endpoint_id.empty()) {
                                g_master_node->GetServer()->SendToClient(actual_endpoint_id, serialized_reply);
                                LOG_INFO_STREAM << "[MSG] 发送通用协议回复: " << reply_payload << " -> " << actual_endpoint_id;
                            }
                        }
                    }
                } else {
                    // 无法解析为协议消息，尝试作为普通文本处理
                    std::string message_str(reinterpret_cast<const char*>(message_data.data()), message_data.size());
                    LOG_INFO_STREAM << "[MSG] 无法解析协议消息，作为文本处理: " << message_str;
                }
            } catch (const std::exception& e) {
                LOG_ERROR_STREAM << "[MSG] 解析消息异常: " << e.what();
            }
        }
    }
    
    void OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) override {
        if (connected) {
            LOG_INFO_STREAM << "[CONN] 客户端连接成功 - 端点: " << endpoint_id
                           << ", 地址: " << connection_info.remote_endpoint.address
                           << ":" << connection_info.remote_endpoint.port;
        } else {
            LOG_INFO_STREAM << "[CONN] 客户端连接断开 - 端点: " << endpoint_id
                           << ", 地址: " << connection_info.remote_endpoint.address
                           << ":" << connection_info.remote_endpoint.port;
        }
    }
    
    void OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) override {
        LOG_ERROR_STREAM << "[ERROR] 连接错误 - 端点: " << endpoint_id 
                         << ", 错误码: " << error_code 
                         << ", 错误信息: " << error_message;
    }
};

int main(int argc, char* argv[]) {
    // 设置信号处理
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    
    // 设置日志级别
    Logger::getInstance().setLevel(Logger::Level::INFO);
    
    std::cout << "=== Master节点示例 ===" << std::endl;
    
    // 加载主配置（单入口）
    auto& config_helper = ConfigHelper::getInstance();
    if (!config_helper.loadConfigFromJson("../../perception_app/config/config.json")) {
        std::cerr << "加载主配置失败，使用默认配置" << std::endl;
    }
    
    // 打印通信配置
    config_helper.printCommunicationConfig();
    
    // 创建Master节点配置
    MasterNode::Config config;
    
    // 从ConfigHelper加载服务发现配置
    config.discovery_address = config_helper.communication_config_.service_discovery.local_address;
    config.discovery_port = config_helper.communication_config_.service_discovery.discovery_port;
    config.broadcast_interval = config_helper.communication_config_.service_discovery.broadcast_interval;
    
    // 从ConfigHelper加载服务器配置
    config.device_server_config.id = config_helper.communication_config_.server.id;
    config.device_server_config.name = config_helper.communication_config_.server.name;
    config.device_server_config.address = config_helper.communication_config_.server.address;
    config.device_server_config.port = config_helper.communication_config_.server.port;
    config.device_server_config.type = EndpointType::Server;
    config.device_server_config.max_clients = config_helper.communication_config_.server.max_clients;
    config.device_server_config.client_timeout = config_helper.communication_config_.server.client_timeout;
    
    // 从ConfigHelper加载Master节点监控配置
    config.client_timeout_interval = config_helper.communication_config_.master_node.client_timeout_interval;
    config.status_check_interval = config_helper.communication_config_.master_node.status_check_interval;
    config.state_sync_interval = config_helper.communication_config_.master_node.state_sync_interval;
    config.enable_auto_cleanup = config_helper.communication_config_.master_node.enable_auto_cleanup;
    config.enable_heartbeat = config_helper.communication_config_.master_node.enable_heartbeat;
    
    std::cout << "客户端监控配置:" << std::endl;
    std::cout << "  超时时间: " << config.client_timeout_interval << "ms" << std::endl;
    std::cout << "  状态检查间隔: " << config.status_check_interval << "ms" << std::endl;
    std::cout << "  自动清理: " << (config.enable_auto_cleanup ? "启用" : "禁用") << std::endl;
    
    // 创建Master节点
    g_master_node = std::make_unique<MasterNode>(config);
    
    // 初始化并启动Master节点
    if (!g_master_node->Initialize()) {
        std::cerr << "Master节点初始化失败!" << std::endl;
        return 1;
    }
    
    // 注册服务端事件处理器
    auto event_handler = std::make_shared<ServerEventHandler>();
    g_master_node->GetServer()->RegisterEventHandler(event_handler);
    
    if (!g_master_node->Start()) {
        std::cerr << "Master节点启动失败!" << std::endl;
        return 1;
    }
    
    std::cout << "Master节点启动成功!" << std::endl;
    std::cout << "按 Ctrl+C 停止..." << std::endl;
    
    // 主循环
    try {
        int loop_count = 0;
        while (g_running) {
            loop_count++;
            // 等待5秒
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    } catch (const std::exception& e) {
        std::cerr << "运行时错误: " << e.what() << std::endl;
    }
    
    // 停止Master节点
    g_master_node->Stop();
    std::cout << "Master节点已停止" << std::endl;
    
    return 0;
}
