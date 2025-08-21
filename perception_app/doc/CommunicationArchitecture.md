# 通信架构（最终版）

## 概述

本架构以“端点（Endpoint）”为中心，彻底解耦服务发现与TCP通信能力，统一配置结构，统一事件处理回调，支持客户端自动重连、服务端端口复用与快速优雅退出。

## 分层结构

```
应用层（App）
├─ 节点层（Nodes）
│  ├─ MasterNode（设备服务器 + 服务发现广播 + EndpointServer）
│  └─ ClientNode（服务发现监听 + DeviceClient + 可选 ControllerClient）
│
├─ 端点层（Endpoints）
│  ├─ EndpointService（统一基类，聚焦TCP连接与消息传输）
│  ├─ EndpointClient（客户端逻辑、自动重连、心跳）
│  └─ EndpointServer（服务端逻辑、连接管理、超时清理）
│
├─ 传输层（Transport）
│  └─ AsioTransport（基于 ASIO 的 TCP 实现，SO_REUSEADDR，线程安全）
│
└─ 服务发现（Service Discovery）
   └─ UdpServiceDiscovery（UDP 广播/接收，按需启动，回调式返回）
```

## 核心统一体

- 统一配置：`EndpointConfig : EndpointIdentity`
  - `EndpointIdentity`：`id/name/address/port/type` + 活动状态（`is_active/last_activity/activity_count`）。
  - `EndpointConfig`：仅保留行为配置（重连、心跳、检查间隔）与服务器专属（`max_clients/client_timeout`）。

- 统一事件：`ITransport::EventHandler`
  - `OnMessageReceived(endpoint_id, data)`
  - `OnConnectionChanged(endpoint_id, connected, ConnectionInfo)`（包含完整远端信息）
  - `OnError(endpoint_id, code, message)`
  - Client/Server/Nodes 直接复用，不再在节点层转发封装。

## 关键设计点

- 服务发现地址来源修复
  - 仅当 JSON 广播中的 `address` 为空或为 `0.0.0.0` 时，才回退使用 `sender.address()`，避免被 WSL/Docker 虚拟网卡地址覆盖。

- 自动重连与心跳
  - Client 运行时通过 `ConnectToServer(address, port)` 指定目标；重连与心跳周期由 `EndpointConfig` 控制。

- 端口复用与快速重启
  - Server 监听端启用 `SO_REUSEADDR`；客户端/服务端停止时主动关闭连接与 IO，上下文线程带 1s 超时退出。

- 优雅退出
  - 监控/心跳线程在 1s 内等待退出，超时分离；IO 线程同策略，避免长时间阻塞。

## 节点层行为

- MasterNode
  - 配置：`device_server_id/name/address/port/max_clients`。
  - 行为：启动服务发现广播；创建 `EndpointServer` 并监听；事件在 `MasterNodeEventHandler` 内直接记录日志。

- ClientNode
  - 配置：`device_client_id/name`、`controller_client_id/name`、`enable_controller_client`。
  - 行为：监听服务发现；发现 `Device Server` 时由 `device_client` 连接；如启用控制器客户端，匹配 `Controller Server` 后连接。
  - 事件：`DeviceClientEventHandler`、`ControllerClientEventHandler` 内直接处理与日志。

## 端点层职责

- EndpointService
  - 聚焦 TCP 连接、消息收发、统计与状态；对外统一接口：`Initialize/Start/Stop/Cleanup/SendMessage/BroadcastMessage`。
  - 统一 `GetConfig()/GetStatistics()/IsRunning()`。

- EndpointClient
  - 运行时 `ConnectToServer(address, port)` 指定目标；支持自动重连、心跳与连接监控。

- EndpointServer
  - 管理客户端表；基于 `max_clients/client_timeout` 控制接入与清理。

## 传输层要点（AsioTransport）

- Server：`acceptor` 绑定 `address:port`，`reuse_address=true`；接受连接后立即上报 `OnConnectionChanged`。
- Client：解析目标并异步连接；连接建立/失败分别上报。
- 统计：消息计数、错误计数、连接计数；统一 JSON 导出。
- 退出：`Stop()` 关闭所有连接、关闭 acceptor、停止 IO；1 秒超时等待 IO 线程退出。

## 服务发现（UdpServiceDiscovery）

- 广播：按 `broadcast_interval` 定时发送本地服务 JSON。
- 接收：独立 IO 线程监听；解析 JSON → 若 `address` 合法则保留，否则回填 `sender.address()`；去重缓存并回调。
- 资源：广播与接收各自持有独立 `io_context/thread/socket`，按需启停，`Stop()` 统一清理。

## 命名与兼容

- 统一命名：`device_client` / `controller_client` / `device_server`。
- 删除：历史 `ServiceInfo/TransportConfig/BaseConfig/ClientConfig/ServerConfig`；统一为 `EndpointIdentity/EndpointConfig`。

## 示例

```cpp
// MasterNode 配置
MasterNode::Config cfg;
cfg.device_server_id = "device_server_001";
cfg.device_server_name = "Device Server";
cfg.device_server_address = "0.0.0.0";
cfg.device_server_port = 9090;
cfg.max_clients = 100;

// ClientNode 配置
ClientNode::Config ccfg;
ccfg.device_client_id = "device_client_001";
ccfg.device_client_name = "Device Client";
ccfg.controller_client_id = "controller_client_001";
ccfg.controller_client_name = "Controller Client";
ccfg.enable_controller_client = true;
```

## 常见问题

- 服务发现到的地址是 172.19.*：这是虚拟网卡地址。已修复为仅在 JSON 中无有效地址时回退使用 sender 地址；确保服务端广播填入可达地址。
- 退出耗时：主要等待监控/心跳/IO 线程退出；已降至 1 秒超时并可强制分离，避免长阻塞。


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
