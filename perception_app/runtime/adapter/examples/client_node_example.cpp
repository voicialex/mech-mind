#include "client_node/ClientNode.hpp"
#include "message/IMessageProtocol.hpp"
#include "Logger.hpp"
#include "configure/ConfigHelper.hpp"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace perception;

std::unique_ptr<ClientNode> g_client_node = nullptr;
std::atomic<bool> g_running{true};
std::atomic<bool> g_connected{false};

void SignalHandler(int signal) {
    std::cout << "\n收到信号 " << signal << "，正在停止Client节点..." << std::endl;
    g_running = false;
    if (g_client_node) {
        g_client_node->Stop();
    }
    exit(0);
}

// 客户端事件处理器
class ClientEventHandler : public ITransport::EventHandler {
public:
    void OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) override {
        LOG_INFO_STREAM << "[MSG] 收到服务端回复 - 端点: " << endpoint_id 
                        << ", 数据大小: " << message_data.size() << " 字节";
        
        // 解析服务端回复（使用消息协议格式）
        if (message_data.size() > 0) {
            try {
                // 尝试解析为协议消息
                auto message = MessageFactory::CreateFromBytes(message_data);
                if (message) {
                    // 成功解析协议消息
                    LOG_INFO_STREAM << "[MSG] 解析协议回复成功 - 类型: " << static_cast<int>(message->GetType())
                                   << ", ID: 0x" << std::hex << message->GetMessageId() << std::dec;
                    
                    // 获取消息负载
                    const auto& frame = message->GetFrame();
                    if (!frame.payload.empty()) {
                        std::string payload_str(frame.payload.begin(), frame.payload.end());
                        LOG_INFO_STREAM << "[MSG] 服务端回复内容: " << payload_str;
                    }
                } else {
                    // 无法解析为协议消息，尝试作为普通文本处理
                    std::string reply_str(reinterpret_cast<const char*>(message_data.data()), message_data.size());
                    LOG_INFO_STREAM << "[MSG] 无法解析协议回复，作为文本处理: " << reply_str;
                }
            } catch (const std::exception& e) {
                LOG_ERROR_STREAM << "[MSG] 解析回复异常: " << e.what();
            }
        }
    }
    
    void OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) override {
        if (connected) {
            LOG_INFO_STREAM << "[CONN] 连接成功 - 端点: " << endpoint_id;
            g_connected = true;
        } else {
            LOG_INFO_STREAM << "[CONN] 连接断开 - 端点: " << endpoint_id;
            g_connected = false;
        }
    }
    
    void OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) override {
        LOG_ERROR_STREAM << "[ERROR] 连接错误 - 端点: " << endpoint_id 
                         << ", 错误码: " << error_code 
                         << ", 错误信息: " << error_message;
    }
};

// 周期性发送端口信息的线程函数
void PortMessageThread() {
    LOG_INFO_STREAM << "[MSG] 启动端口信息发送线程";
    
    while (g_running) {
        if (g_connected && g_client_node) {
            try {
                // 构建端口信息消息（使用消息协议格式）
                auto message = MessageFactory::CreateMessage(
                    MessageType::Notify, 
                    MessageIds::DEVICE_STATUS, 
                    SubMessageIds::READY
                );
                
                // 设置端口信息作为负载
                std::string port_info = "CLIENT_PORT_INFO:8081";
                std::vector<uint8_t> payload(port_info.begin(), port_info.end());
                
                // 获取并修改消息帧
                ProtocolFrame frame = message->GetFrame();
                frame.payload = payload;
                frame.length = payload.size();
                message->SetFrame(frame);
                
                // 序列化消息
                std::vector<uint8_t> message_data = message->Serialize();
                
                // 发送消息到服务端
                if (g_client_node->GetDeviceClient()->SendMessage("", message_data)) {
                    LOG_INFO_STREAM << "[MSG] 发送端口信息成功: " << port_info;
                } else {
                    LOG_WARNING_STREAM << "[MSG] 发送端口信息失败";
                }
            } catch (const std::exception& e) {
                LOG_ERROR_STREAM << "[MSG] 发送端口信息异常: " << e.what();
            }
        }
        
        // 等待3秒
        for (int i = 0; i < 30 && g_running; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    LOG_INFO_STREAM << "[MSG] 端口信息发送线程已退出";
}

int main(int argc, char* argv[]) {
    // 设置信号处理
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    
    // 设置日志级别
    Logger::getInstance().setLevel(Logger::Level::INFO);
    
    std::cout << "=== Client节点示例 ===" << std::endl;
    
    // 加载主配置（单入口）
    auto& config_helper = ConfigHelper::getInstance();
    if (!config_helper.loadConfigFromJson("../../perception_app/config/config.json")) {
        std::cerr << "加载主配置失败，使用默认配置" << std::endl;
    }
    
    // 打印通信配置
    config_helper.printCommunicationConfig();
    
    // 创建Client节点配置
    ClientNode::Config config;
    
    // 从ConfigHelper加载服务发现配置
    config.discovery_address = config_helper.communication_config_.service_discovery.local_address;
    config.discovery_port = config_helper.communication_config_.service_discovery.discovery_port;
    
    // 从ConfigHelper加载客户端配置
    config.device_client_id = config_helper.communication_config_.client.id;
    config.device_client_name = config_helper.communication_config_.client.name;
    config.controller_client_id = "controller_client_001";
    config.controller_client_name = "Controller Client";
    config.enable_controller_client = false;
    
    // 创建Client节点
    g_client_node = std::make_unique<ClientNode>(config);
    
    // 初始化并启动Client节点
    if (!g_client_node->Initialize()) {
        std::cerr << "Client节点初始化失败!" << std::endl;
        return 1;
    }
    
    // 注册事件处理器（在初始化之后）
    auto event_handler = std::make_shared<ClientEventHandler>();
    g_client_node->GetDeviceClient()->RegisterEventHandler(event_handler);
    
    if (!g_client_node->Start()) {
        std::cerr << "Client节点启动失败!" << std::endl;
        return 1;
    }
    
    std::cout << "Client节点启动成功!" << std::endl;
    std::cout << "正在等待连接Master服务器..." << std::endl;
    std::cout << "按 Ctrl+C 停止..." << std::endl;
    
    // 启动端口信息发送线程
    std::thread port_thread(PortMessageThread);
    
    // 主循环
    try {
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    } catch (const std::exception& e) {
        std::cerr << "运行时错误: " << e.what() << std::endl;
    }
    
    // 等待端口信息发送线程结束
    if (port_thread.joinable()) {
        port_thread.join();
    }
    
    // 停止Client节点
    g_client_node->Stop();
    std::cout << "Client节点已停止" << std::endl;
    
    return 0;
}
