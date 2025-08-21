#pragma once

#include "ConnectionTypes.hpp"
#include "ITransport.hpp"
#include <memory>
#include <string>
#include <functional>
#include <vector>

namespace perception {

/**
 * @brief 端点服务接口 - 统一的端点服务抽象
 * 
 * 设计原则：
 * - 提供统一的端点服务接口
 * - 支持客户端和服务器模式
 * - 与外部消息协议解耦
 * - 只定义接口，不包含具体实现
 * - 统一的事件处理器管理
 */
class IEndpointService {
public:
    using Ptr = std::shared_ptr<IEndpointService>;
    
    /**
     * @brief 端点状态
     */
    enum class EndpointState {
        Stopped,
        Starting,
        Running,
        Stopping,
        Error
    };
    
    /**
     * @brief 统一的事件处理器接口 - 使用ITransport::EventHandler
     */
    using EventHandler = ITransport::EventHandler;

public:
    virtual ~IEndpointService() = default;
    
    /**
     * @brief 初始化端点服务
     * @return 是否初始化成功
     */
    virtual bool Initialize() = 0;
    
    /**
     * @brief 启动端点服务
     * @return 是否启动成功
     */
    virtual bool Start() = 0;
    
    /**
     * @brief 停止端点服务
     */
    virtual void Stop() = 0;
    
    /**
     * @brief 清理资源
     */
    virtual void Cleanup() = 0;
    
    /**
     * @brief 发送消息
     * @param target_id 目标ID
     * @param message_data 消息数据
     * @param timeout_ms 超时时间（毫秒），0表示不等待响应
     * @return 是否发送成功
     */
    virtual bool SendMessage(const std::string& target_id, const std::vector<uint8_t>& message_data, uint32_t timeout_ms = 0) = 0;
    
    /**
     * @brief 广播消息
     * @param message_data 消息数据
     * @param target_name 目标名称（可选）
     */
    virtual void BroadcastMessage(const std::vector<uint8_t>& message_data, 
                                const std::string& target_name = "") = 0;
    
    /**
     * @brief 注册事件处理器
     * @param handler 事件处理器
     */
    virtual void RegisterEventHandler(EventHandler::Ptr handler) = 0;
    
    /**
     * @brief 检查端点是否在线
     * @param endpoint_id 端点ID
     * @return 是否在线
     */
    virtual bool IsEndpointOnline(const std::string& endpoint_id) const = 0;
    
    /**
     * @brief 获取端点状态
     * @return 端点状态
     */
    virtual EndpointState GetState() const = 0;
    
    /**
     * @brief 获取配置
     * @return 配置
     */
    virtual const EndpointConfig& GetConfig() const = 0;
    
    /**
     * @brief 检查是否正在运行
     * @return 是否正在运行
     */
    virtual bool IsRunning() const = 0;
};

} // namespace perception
