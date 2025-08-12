# 感知应用通信架构设计

## 概述

本文档描述了基于ASIO库设计的感知应用通信架构，该架构实现了独立的通信层和消息层，为runtime提供统一的通信能力，支持感知应用的网络化部署和扩展。

## 架构设计

### 核心组件

```
┌─────────────────────────────────────────────────────────────┐
│                    感知应用通信架构                           │
├─────────────────────────────────────────────────────────────┤
│  Runtime Layer (运行时层)                                    │
│  ├── CommunicationInterface (通信接口)                       │
│  ├── CommunicationInterfaceManager (通信接口管理器)          │
│  └── Runtime Integration (运行时集成)                        │
├─────────────────────────────────────────────────────────────┤
│  Communication Layer (通信层)                                │
│  ├── CommunicationManager (通信管理器)                       │
│  ├── TcpConnection (TCP连接管理)                             │
│  ├── UdpDiscovery (UDP服务发现)                              │
│  └── MessageProcessor (消息处理器)                           │
├─────────────────────────────────────────────────────────────┤
│  Message Layer (消息层)                                      │
│  ├── MessageProtocol (消息协议)                              │
│  ├── PerceptionMessages (感知消息)                           │
│  ├── MessageFactory (消息工厂)                               │
│  └── MessageRouter (消息路由器)                              │
├─────────────────────────────────────────────────────────────┤
│  Transport Layer (传输层)                                    │
│  ├── ASIO (异步网络库)                                       │
│  ├── TCP/UDP Sockets (套接字)                                │
│  └── Serialization (序列化)                                  │
└─────────────────────────────────────────────────────────────┘
```

### 设计原则

1. **独立性**: 通信层只关注通信功能，不耦合业务逻辑
2. **模块化设计**: 各层职责清晰，接口明确
3. **异步通信**: 基于ASIO实现高性能异步通信
4. **服务发现**: 自动发现和管理网络服务
5. **消息传输**: 提供通用的消息传输接口
6. **错误处理**: 完善的错误处理和恢复机制
7. **可扩展性**: 支持插件式扩展和配置

## 详细设计

### 1. 通信管理器 (CommunicationManager)

**职责:**
- 服务发现和注册
- 连接管理
- 消息传输
- 异步通信处理

**主要功能:**
```cpp
class CommunicationManager {
    // 服务管理
    bool RegisterService(const ServiceInfo& service_info);
    std::vector<ServiceInfo> DiscoverServices(const std::string& service_name);
    
    // 连接管理
    bool ConnectToService(const std::string& service_id);
    void DisconnectFromService(const std::string& service_id);
    
    // 消息发送
    bool SendMessage(const std::string& target_id, const std::vector<uint8_t>& data);
    std::vector<uint8_t> SendRequest(const std::string& target_id, 
                                    const std::vector<uint8_t>& data,
                                    uint32_t timeout_ms);
};
```

### 2. 消息协议 (MessageProtocol)

**协议格式:**
```
┌─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
│ Magic   │ CRC16   │ MsgType │ MsgID   │ SubMsgID│ Sequence│
│ (2字节)  │ (2字节)  │ (1字节)  │ (2字节)  │ (1字节)  │ (2字节)  │
├─────────┴─────────┴─────────┴─────────┴─────────┴─────────┤
│ Length  │ Payload                                         │
│ (2字节)  │ (变长)                                          │
└───────────────────────────────────────────────────────────┘
```

**消息类型:**
- Request (0x00): 请求消息
- Response (0x01): 响应消息
- Notify (0x02): 通知消息

### 3. 感知消息 (PerceptionMessages)

**消息ID定义:**
```cpp
namespace MessageIds {
    // 系统消息 (0x0001-0x00FF)
    static constexpr uint16_t HEARTBEAT = 0x0001;
    static constexpr uint16_t SERVICE_DISCOVERY = 0x0002;
    
    // 感知应用消息 (0x0100-0x01FF)
    static constexpr uint16_t PERCEPTION_START = 0x0100;
    static constexpr uint16_t PERCEPTION_STOP = 0x0101;
    static constexpr uint16_t PERCEPTION_STATUS = 0x0102;
    
    // 推理相关消息 (0x0200-0x02FF)
    static constexpr uint16_t INFERENCE_REQUEST = 0x0200;
    static constexpr uint16_t INFERENCE_RESPONSE = 0x0201;
    static constexpr uint16_t INFERENCE_STATUS = 0x0202;
}
```

### 4. 通信接口 (CommunicationInterface)

**功能特性:**
- 为runtime提供统一的通信能力
- 支持同步和异步通信模式
- 自动处理连接管理和错误恢复
- 提供简单的消息发送和接收接口

**使用示例:**
```cpp
// 创建通信接口
CommunicationInterface::Config config;
config.local_service_id = "client_001";
config.local_service_name = "Perception Client";

auto comm_interface = std::make_unique<CommunicationInterface>(config);

// 设置回调
comm_interface->RegisterMessageCallback([](const Message::Ptr& message) {
    std::cout << "收到消息: " << message->ToString() << std::endl;
});

// 初始化并连接
comm_interface->Initialize();
comm_interface->Start();
comm_interface->ConnectToService("target_service");

// 发送消息
auto message = std::make_shared<PerceptionStartMessage>();
comm_interface->SendMessage("target_service", message);
```

## 配置管理

### 通信配置文件 (communication_config.json)

```json
{
    "communication": {
        "local_service": {
            "service_id": "perception_app_001",
            "service_name": "Perception Application",
            "version": "1.0.0"
        },
        "network": {
            "local_address": "0.0.0.0",
            "local_port": 8080,
            "discovery_port": 8081,
            "max_connections": 50
        },
        "inference_service": {
            "service_name": "Inference Service",
            "auto_discovery": true,
            "request_timeout_ms": 10000
        }
    }
}
```

## 部署架构

### 分布式部署模式

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   感知应用客户端   │    │   推理服务节点   │    │   相机服务节点   │
│                 │    │                 │    │                 │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │ 通信管理器   │ │◄──►│ │ 通信管理器   │ │    │ │ 通信管理器   │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │ 推理接口    │ │    │ │ 推理引擎    │ │    │ │ 相机控制    │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### 服务发现流程

1. **服务注册**: 各节点启动时注册服务信息
2. **服务发现**: 客户端通过UDP广播发现可用服务
3. **连接建立**: 客户端与目标服务建立TCP连接
4. **心跳检测**: 定期发送心跳包检测连接状态
5. **自动重连**: 连接断开时自动重连

## 性能优化

### 1. 异步处理
- 使用ASIO的异步I/O模型
- 非阻塞消息处理
- 线程池管理

### 2. 连接池
- 复用TCP连接
- 连接状态管理
- 负载均衡

### 3. 消息压缩
- 大数据压缩传输
- 压缩阈值配置
- 压缩算法选择

### 4. 缓存机制
- 消息缓存
- 结果缓存
- 连接缓存

## 错误处理

### 错误码定义

```cpp
namespace ErrorCodes {
    static constexpr uint16_t SUCCESS = 0x0000;
    static constexpr uint16_t GENERAL_ERROR = 0x0001;
    static constexpr uint16_t TIMEOUT = 0x0003;
    static constexpr uint16_t INFERENCE_FAILED = 0x0201;
    static constexpr uint16_t CAMERA_DISCONNECTED = 0x0303;
}
```

### 错误恢复策略

1. **连接错误**: 自动重连机制
2. **超时错误**: 重试机制
3. **服务错误**: 服务切换
4. **系统错误**: 降级处理

## 安全考虑

### 1. 认证机制
- 服务认证
- 消息认证
- 权限控制

### 2. 加密传输
- TLS/SSL加密
- 消息加密
- 密钥管理

### 3. 访问控制
- IP白名单
- 服务白名单
- 操作权限

## 监控和日志

### 1. 性能监控
- 连接状态监控
- 消息吞吐量监控
- 响应时间监控

### 2. 日志记录
- 网络日志
- 消息日志
- 错误日志

### 3. 健康检查
- 服务健康检查
- 连接健康检查
- 系统健康检查

## 扩展性设计

### 1. 插件机制
- 消息处理器插件
- 协议插件
- 传输插件

### 2. 配置热更新
- 动态配置更新
- 服务配置更新
- 路由配置更新

### 3. 水平扩展
- 负载均衡
- 服务集群
- 数据分片

## 总结

该通信架构为感知应用提供了完整的网络通信解决方案，具有以下特点：

1. **高性能**: 基于ASIO的异步I/O模型
2. **高可靠**: 完善的错误处理和自动恢复机制
3. **易扩展**: 模块化设计和插件机制
4. **易使用**: 简洁的API接口和丰富的示例
5. **易部署**: 灵活的配置管理和服务发现

该架构可以支持感知应用的分布式部署，实现推理服务的网络化调用，为大规模感知应用提供了可靠的通信基础。
