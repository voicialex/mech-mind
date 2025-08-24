#pragma once

#include "ProtocolDefinitions.hpp"
#include "communication/interfaces/ITransport.hpp"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace perception {

/**
 * @brief 消息回调函数类型定义
 */
using MessageCallback = std::function<void(std::shared_ptr<ITransport> transport, uint16_t message_id, uint8_t sub_message_id, 
                                         const std::vector<uint8_t>& payload)>;

/**
 * @brief 消息路由键结构体
 */
struct MessageKey {
    MessageType message_type;
    uint16_t message_id;
    uint8_t sub_message_id;
    
    MessageKey(MessageType type, uint16_t id, uint8_t sub_id) 
        : message_type(type), message_id(id), sub_message_id(sub_id) {}
    
    // 用于unordered_map的哈希函数
    bool operator==(const MessageKey& other) const {
        return message_type == other.message_type && 
               message_id == other.message_id && 
               sub_message_id == other.sub_message_id;
    }
};

/**
 * @brief 消息路由键哈希函数
 */
struct MessageKeyHash {
    std::size_t operator()(const MessageKey& key) const {
        std::size_t h1 = std::hash<uint8_t>{}(static_cast<uint8_t>(key.message_type));
        std::size_t h2 = std::hash<uint16_t>{}(key.message_id);
        std::size_t h3 = std::hash<uint8_t>{}(key.sub_message_id);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

/**
 * @brief 消息路由器类
 */
class MessageRouter {
public:
    using Ptr = std::shared_ptr<MessageRouter>;
    
    /**
     * @brief 注册消息回调函数
     * @param message_type 消息类型
     * @param message_id 消息ID
     * @param sub_message_id 子消息ID
     * @param callback 回调函数
     */
    void RegisterCallback(MessageType message_type, uint16_t message_id, uint8_t sub_message_id, 
                         MessageCallback callback);
    
    /**
     * @brief 注销消息回调函数
     * @param message_type 消息类型
     * @param message_id 消息ID
     * @param sub_message_id 子消息ID
     */
    void UnregisterCallback(MessageType message_type, uint16_t message_id, uint8_t sub_message_id);
    
    /**
     * @brief 路由消息到对应的回调函数
     * @param transport 传输层实例
     * @param message_type 消息类型
     * @param message_id 消息ID
     * @param sub_message_id 子消息ID
     * @param payload 负载数据
     * @return 是否找到并执行了回调函数
     */
    bool InvokeCallback(std::shared_ptr<ITransport> transport, MessageType message_type, uint16_t message_id, uint8_t sub_message_id,
                     const std::vector<uint8_t>& payload);
    
    /**
     * @brief 检查是否有对应的回调函数
     * @param message_type 消息类型
     * @param message_id 消息ID
     * @param sub_message_id 子消息ID
     * @return 是否有回调函数
     */
    bool HasCallback(MessageType message_type, uint16_t message_id, uint8_t sub_message_id) const;
    
    /**
     * @brief 获取注册的回调函数数量
     * @return 回调函数数量
     */
    size_t GetCallbackCount() const;
    
    /**
     * @brief 清空所有回调函数
     */
    void Clear();
    
    /**
     * @brief 初始化默认的消息路由表（硬编码绑定）
     */
    void InitializeDefaultRoutes();

private:
    std::unordered_map<MessageKey, MessageCallback, MessageKeyHash> callbacks_;
    mutable std::mutex router_mutex_;
};

/**
 * @brief 基础消息接口
 */
class IMessage {
public:
    using Ptr = std::shared_ptr<IMessage>;
    
    virtual ~IMessage() = default;
    
    /**
     * @brief 获取消息类型
     * @return 消息类型
     */
    virtual MessageType GetType() const = 0;
    
    /**
     * @brief 获取消息ID
     * @return 消息ID
     */
    virtual uint16_t GetMessageId() const = 0;
    
    /**
     * @brief 获取子消息ID
     * @return 子消息ID
     */
    virtual uint8_t GetSubMessageId() const = 0;
    
    /**
     * @brief 获取序列号
     * @return 序列号
     */
    virtual uint16_t GetSequence() const = 0;
    
    /**
     * @brief 获取时间戳
     * @return 时间戳
     */
    virtual uint64_t GetTimestamp() const = 0;
    
    /**
     * @brief 获取消息状态
     * @return 消息状态
     */
    virtual MessageStatus GetStatus() const = 0;
    
    /**
     * @brief 设置消息状态
     * @param status 消息状态
     */
    virtual void SetStatus(MessageStatus status) = 0;
    
    /**
     * @brief 序列化消息
     * @return 序列化后的字节数组
     */
    virtual std::vector<uint8_t> Serialize() const = 0;
    
    /**
     * @brief 反序列化消息
     * @param data 字节数组
     * @return 是否反序列化成功
     */
    virtual bool Deserialize(const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief 验证消息完整性
     * @return 是否有效
     */
    virtual bool Validate() const = 0;
    
    /**
     * @brief 转换为字符串表示
     * @return 字符串表示
     */
    virtual std::string ToString() const = 0;
    
    /**
     * @brief 获取协议帧
     * @return 协议帧结构
     */
    virtual const ProtocolFrame& GetFrame() const = 0;
    
    /**
     * @brief 设置协议帧
     * @param frame 协议帧结构
     */
    virtual void SetFrame(const ProtocolFrame& frame) = 0;
    
    /**
     * @brief 处理消息（自动路由到对应的回调函数）
     * @param transport 传输层实例
     * @param router 消息路由器
     * @return 是否成功处理
     */
    virtual bool ProcessMessage(std::shared_ptr<ITransport> transport, std::shared_ptr<MessageRouter> router) = 0;
};

/**
 * @brief 消息工厂类
 */
class MessageFactory {
public:
    /**
     * @brief 从字节数组创建消息
     * @param data 字节数组
     * @return 消息指针
     */
    static IMessage::Ptr CreateFromBytes(const std::vector<uint8_t>& data);
    
    /**
     * @brief 验证消息格式
     * @param data 字节数组
     * @return 是否有效
     */
    static bool ValidateMessage(const std::vector<uint8_t>& data);
    
    /**
     * @brief 创建心跳请求消息（服务器发起）
     * @return 心跳请求消息数据
     */
    static std::vector<uint8_t> CreateHeartbeatRequestMessage();
    
    /**
     * @brief 创建心跳响应消息（客户端回复）
     * @return 心跳响应消息数据
     */
    static std::vector<uint8_t> CreateHeartbeatResponseMessage();
    
    /**
     * @brief 检查是否是心跳请求消息
     * @param message_data 消息数据
     * @return 是否是心跳请求消息
     */
    static bool IsHeartbeatRequestMessage(const std::vector<uint8_t>& message_data);
    
    /**
     * @brief 检查是否是心跳响应消息
     * @param message_data 消息数据
     * @return 是否是心跳响应消息
     */
    static bool IsHeartbeatResponseMessage(const std::vector<uint8_t>& message_data);
    
    /**
     * @brief 检查是否是心跳消息（请求或响应）
     * @param message_data 消息数据
     * @return 是否是心跳消息
     */
    static bool IsHeartbeatMessage(const std::vector<uint8_t>& message_data);
    
    /**
     * @brief 注册消息回调函数
     * @param message_id 消息ID
     * @param callback 回调函数
     */
    static void RegisterMessageCallback(uint16_t message_id, MessageCallback callback);
    
    /**
     * @brief 注销消息回调函数
     * @param message_id 消息ID
     */
    static void UnregisterMessageCallback(uint16_t message_id);
    
    /**
     * @brief 处理消息数据（自动创建消息对象并路由）
     * @param transport 传输层实例
     * @param router 消息路由器
     * @param message_data 消息数据
     * @return 是否成功处理
     */
    static bool ProcessMessageData(std::shared_ptr<ITransport> transport, std::shared_ptr<MessageRouter> router,
                                  const std::vector<uint8_t>& message_data);
    
    /**
     * @brief 创建请求消息
     * @param message_id 消息ID
     * @param sub_message_id 子消息ID
     * @param payload 负载数据
     * @return 序列化后的消息数据
     */
    static std::vector<uint8_t> CreateRequestMessage(uint16_t message_id, uint8_t sub_message_id, 
                                                    const std::vector<uint8_t>& payload);
    
    /**
     * @brief 创建响应消息
     * @param message_id 消息ID
     * @param sub_message_id 子消息ID
     * @param payload 负载数据
     * @return 序列化后的消息数据
     */
    static std::vector<uint8_t> CreateResponseMessage(uint16_t message_id, uint8_t sub_message_id, 
                                                     const std::vector<uint8_t>& payload);
    
    /**
     * @brief 创建通知消息
     * @param message_id 消息ID
     * @param sub_message_id 子消息ID
     * @param payload 负载数据
     * @return 序列化后的消息数据
     */
    static std::vector<uint8_t> CreateNotifyMessage(uint16_t message_id, uint8_t sub_message_id, 
                                                   const std::vector<uint8_t>& payload);
    
    /**
     * @brief 创建请求消息（字符串负载）
     * @param message_id 消息ID
     * @param sub_message_id 子消息ID
     * @param payload_str 负载字符串
     * @return 序列化后的消息数据
     */
    static std::vector<uint8_t> CreateRequestMessage(uint16_t message_id, uint8_t sub_message_id, 
                                                    const std::string& payload_str);
    
    /**
     * @brief 创建响应消息（字符串负载）
     * @param message_id 消息ID
     * @param sub_message_id 子消息ID
     * @param payload_str 负载字符串
     * @return 序列化后的消息数据
     */
    static std::vector<uint8_t> CreateResponseMessage(uint16_t message_id, uint8_t sub_message_id, 
                                                     const std::string& payload_str);
    
    /**
     * @brief 创建通知消息（字符串负载）
     * @param message_id 消息ID
     * @param sub_message_id 子消息ID
     * @param payload_str 负载字符串
     * @return 序列化后的消息数据
     */
    static std::vector<uint8_t> CreateNotifyMessage(uint16_t message_id, uint8_t sub_message_id, 
                                                   const std::string& payload_str);
    
    /**
     * @brief 获取消息类型名称
     * @param message_id 消息ID
     * @return 消息类型名称
     */
    static std::string GetMessageTypeName(uint16_t message_id);
    
    /**
     * @brief 获取子消息类型名称
     * @param sub_message_id 子消息ID
     * @return 子消息类型名称
     */
    static std::string GetSubMessageTypeName(uint8_t sub_message_id);
    
    /**
     * @brief 获取错误码描述
     * @param error_code 错误码
     * @return 错误码描述
     */
    static std::string GetErrorCodeDescription(uint16_t error_code);

private:
    static std::unordered_map<uint16_t, MessageCallback> message_callbacks_;
    static std::mutex factory_mutex_;
};

} // namespace perception
