#include "DeviceServer.hpp"
#include <iostream>
#include <signal.h>
#include <memory>
#include <thread>
#include <chrono>

using namespace device_center;

// 全局变量
std::unique_ptr<DeviceServer> g_server;
std::atomic<bool> g_running{true};

// 信号处理函数
void SignalHandler(int signal) {
    std::cout << "\n收到信号 " << signal << "，正在停止服务器..." << std::endl;
    g_running = false;
}

// 统一的设备事件处理器实现
class UnifiedDeviceEventHandler : public DeviceEventHandler {
public:
    void OnDeviceConnectionChanged(const std::string& device_id, bool connected) override {
        std::cout << "设备连接状态变化: " << device_id << " " << (connected ? "已连接" : "已断开") << std::endl;
    }

    void OnDeviceStatusChanged(const std::string& device_id, DeviceStatus old_status, DeviceStatus new_status) override {
        std::cout << "设备状态变化: " << device_id << " " 
                  << DeviceUtils::StatusToString(old_status) << " -> " 
                  << DeviceUtils::StatusToString(new_status) << std::endl;
    }

    void OnDeviceDataReceived(const DeviceData& device_data) override {
        std::cout << "收到设备数据: " << device_data.device_id 
                  << " 类型: " << device_data.data_type 
                  << " 大小: " << device_data.data_size << std::endl;
    }

    DeviceResponse OnDeviceCommand(const DeviceCommand& command) override {
        std::cout << "处理设备命令: " << command.device_id 
                  << " 类型: " << command.command_type << std::endl;
        
        // 示例命令处理逻辑
        DeviceResponse response;
        response.device_id = command.device_id;
        response.command_id = command.command_id;
        response.timestamp = DeviceUtils::GetCurrentTimestamp();
        
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
        
        return response;
    }

    void OnDeviceError(const std::string& device_id, uint16_t error_code, const std::string& error_message) override {
        std::cout << "设备错误: " << device_id 
                  << " 错误码: " << error_code 
                  << " 消息: " << error_message << std::endl;
    }
};

int main(int argc, char *argv[]) {
    std::cout << "=== Device Center Server (优化版) ===" << std::endl;

    // 设置信号处理
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    // 配置文件路径
    std::string config_path = "config/server_config.json";
    if (argc > 1) {
        config_path = argv[1];
    }

    try {
        // 创建设备服务器
        g_server = std::make_unique<DeviceServer>();

        // 初始化服务器
        std::cout << "正在初始化设备服务器..." << std::endl;
        if (!g_server->Initialize(config_path)) {
            std::cerr << "服务器初始化失败!" << std::endl;
            return -1;
        }

        // 注册统一的事件处理器
        std::cout << "正在注册事件处理器..." << std::endl;
        auto event_handler = std::make_shared<UnifiedDeviceEventHandler>();
        g_server->RegisterEventHandler(event_handler);

        // 启动服务器
        std::cout << "正在启动设备服务器..." << std::endl;
        if (!g_server->Start()) {
            std::cerr << "服务器启动失败!" << std::endl;
            return -1;
        }

        std::cout << "设备服务器启动成功!" << std::endl;
        std::cout << "服务器ID: " << g_server->GetConfig().service_id << std::endl;
        std::cout << "监听端口: " << g_server->GetConfig().local_port << std::endl;
        std::cout << "按 Ctrl+C 停止服务器" << std::endl;

        // 运行服务器循环
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // 定期输出统计信息
            static int counter = 0;
            if (++counter % 30 == 0) {  // 每30秒输出一次统计信息
                auto stats = g_server->GetServerStatistics();
                std::cout << "服务器统计: " << stats.dump(2) << std::endl;

                auto devices = g_server->GetAllDevices();
                std::cout << "已注册设备数量: " << devices.size() << std::endl;

                auto online_devices = g_server->GetOnlineDevices();
                std::cout << "在线设备数量: " << online_devices.size() << std::endl;
            }
        }

        // 停止服务器
        std::cout << "正在停止设备服务器..." << std::endl;
        g_server->Stop();
        g_server->Cleanup();

        std::cout << "设备服务器已停止" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "服务器运行异常: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
