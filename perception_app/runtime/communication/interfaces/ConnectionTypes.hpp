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
    
    // 活动状态（端点级别的统计信息）
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
        oss << indent << "  last_activity: " << last_activity << "\n";
        oss << indent << "  activity_count: " << activity_count << "\n";
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

} // namespace perception
