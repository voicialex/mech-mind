#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <sstream>

namespace perception {

/**
 * @brief 连接状态枚举
 */
enum class ConnectionState {
    Disconnected,  // 未连接
    Connecting,    // 连接中
    Connected,     // 已连接
    Error          // 错误状态
};

/**
 * @brief 端点类型枚举
 */
enum class EndpointType {
    Client,  // 客户端端点
    Server   // 服务器端点
};

// 统一端点身份与活动状态
struct EndpointIdentity {
    // 身份信息
    std::string id;
    std::string name;
    std::string address;
    uint16_t port{0};
    EndpointType type{EndpointType::Client};
    
    // 活动状态
    bool is_active{false};         // 是否活跃/在线/已连接
    uint64_t last_activity{0};     // 最后活动时间戳
    uint32_t activity_count{0};    // 活动计数（消息数/心跳数等）
    
    /**
     * @brief 调试打印方法
     * @param indent 缩进字符串
     * @return 格式化的调试信息字符串
     */
    std::string DebugPrint(const std::string& indent = "") const {
        std::ostringstream oss;
        oss << indent << "EndpointIdentity {\n";
        oss << indent << "  id: \"" << id << "\"\n";
        oss << indent << "  name: \"" << name << "\"\n";
        oss << indent << "  address: \"" << address << "\"\n";
        oss << indent << "  port: " << port << "\n";
        oss << indent << "  type: " << (type == EndpointType::Client ? "Client" : "Server") << "\n";
        oss << indent << "  is_active: " << (is_active ? "true" : "false") << "\n";
        oss << indent << "  last_activity: " << last_activity << "\n";
        oss << indent << "  activity_count: " << activity_count << "\n";
        oss << indent << "}";
        return oss.str();
    }
};

/**
 * @brief 端点配置结构体
 */
struct EndpointConfig : public EndpointIdentity {
    // 客户端特有配置（仅行为相关，不含目标地址）
    bool enable_auto_reconnect{true};  // 是否启用自动重连
    uint32_t max_reconnect_attempts{10}; // 最大重连次数（0表示无限）
    uint32_t reconnect_interval{5000}; // 重连间隔（毫秒）
    uint32_t heartbeat_interval{30000}; // 心跳间隔（毫秒）
    uint32_t connection_check_interval{10000}; // 连接状态检查间隔（毫秒）

    // 服务器特有配置
    uint32_t max_clients{100};         // 最大客户端数
    uint32_t client_timeout{30000};    // 客户端超时（毫秒）
    
    std::string DebugPrint(const std::string& indent = "") const {
        std::ostringstream oss;
        oss << EndpointIdentity::DebugPrint(indent);
        // 客户端行为配置
        oss << indent << "  // 客户端行为配置:\n";
        oss << indent << "  enable_auto_reconnect: " << (enable_auto_reconnect ? "true" : "false") << "\n";
        oss << indent << "  max_reconnect_attempts: " << max_reconnect_attempts << "\n";
        oss << indent << "  reconnect_interval: " << reconnect_interval << "ms\n";
        oss << indent << "  heartbeat_interval: " << heartbeat_interval << "ms\n";
        oss << indent << "  connection_check_interval: " << connection_check_interval << "ms\n";
        
        // 服务器配置
        oss << indent << "  // 服务器配置:\n";
        oss << indent << "  max_clients: " << max_clients << "\n";
        oss << indent << "  client_timeout: " << client_timeout << "ms\n";
        oss << indent << "}";
        return oss.str();
    }
};

/**
 * @brief 连接信息结构体
 */
struct ConnectionInfo {
    EndpointIdentity local_endpoint;   // 本地端点信息
    EndpointIdentity remote_endpoint;  // 远程端点信息
    ConnectionState state;             // 连接状态
    uint64_t connect_time;             // 连接时间戳
    uint32_t reconnect_attempts;       // 重连尝试次数
    
    ConnectionInfo() : state(ConnectionState::Disconnected), 
                      connect_time(0), reconnect_attempts(0) {}
};

/**
 * @brief 连接状态回调函数类型
 */
using ConnectionStateCallback = std::function<void(const std::string&, ConnectionState)>;

/**
 * @brief 连接布尔回调函数类型
 */
using ConnectionBoolCallback = std::function<void(const std::string&, bool)>;

/**
 * @brief 连接工具函数命名空间
 */
namespace ConnectionUtils {
    /**
     * @brief 检查连接状态是否为已连接
     * @param state 连接状态
     * @return 是否已连接
     */
    inline bool IsConnected(ConnectionState state) {
        return state == ConnectionState::Connected;
    }
    
    /**
     * @brief 从布尔值转换为连接状态
     * @param connected 是否连接
     * @return 连接状态
     */
    inline ConnectionState FromBool(bool connected) {
        return connected ? ConnectionState::Connected : ConnectionState::Disconnected;
    }
    
    /**
     * @brief 连接状态转字符串
     * @param state 连接状态
     * @return 状态字符串
     */
    inline std::string ToString(ConnectionState state) {
        switch (state) {
            case ConnectionState::Disconnected: return "Disconnected";
            case ConnectionState::Connecting: return "Connecting";
            case ConnectionState::Connected: return "Connected";
            case ConnectionState::Error: return "Error";
            default: return "Unknown";
        }
    }
}

} // namespace perception
