# Endpoint 模块架构总结

## 概述

新的架构设计将通信层、协议层和实现层进行了清晰的分离，实现了更好的模块化和可扩展性。

## 架构层次

### 1. Communication 模块（接口层）
- **位置**: `perception_app/runtime/communication/`
- **职责**: 只定义接口，不包含具体实现
- **核心接口**:
  - `IEndpointService`: 统一的端点服务接口
  - `IEndpointServiceFactory`: 端点服务工厂接口
  - `ICommunicationService`: 通信服务接口
  - `ITransport`: 传输层接口
  - `IServiceDiscovery`: 服务发现接口

### 2. Endpoint 模块（实现层）
- **位置**: `perception_app/runtime/endpoint/`
- **职责**: 实现具体的客户端和服务器功能
- **核心组件**:
  - `EndpointService`: 基础端点服务实现
  - `EndpointClient`: 客户端实现
  - `EndpointServer`: 服务器实现
  - `MessageProtocolAdapter`: 消息协议适配器

### 3. Message 模块（协议层）
- **位置**: `perception_app/runtime/message/`
- **职责**: 定义和实现具体的消息协议
- **核心组件**:
  - `MessageProtocol.hpp/cpp`: 基础消息协议
  - `PerceptionMessages.hpp`: 感知应用消息定义

## 设计原则

### 1. 分层设计
```
┌─────────────────────────────────────┐
│           Application Layer         │ 应用层
├─────────────────────────────────────┤
│         Message Protocol Layer      │ 消息协议层
├─────────────────────────────────────┤
│         Endpoint Layer              │ 端点层
├─────────────────────────────────────┤
│      Communication Interface        │ 通信接口层
├─────────────────────────────────────┤
│         Transport Layer             │ 传输层
└─────────────────────────────────────┘
```

### 2. 协议隔离
- Communication 模块只定义接口，不包含具体协议实现
- 具体的消息协议由外部的 Message 模块实现
- 通过 MessageProtocolAdapter 实现协议适配和转换

### 3. 模块化设计
- 每个模块职责单一，边界清晰
- 模块间通过接口进行交互
- 支持插件式扩展

## 核心组件详解

### IEndpointService 接口
```cpp
class IEndpointService {
public:
    enum class EndpointType { Client, Server };
    enum class EndpointState { Stopped, Starting, Running, Stopping, Error };
    
    virtual bool Initialize() = 0;
    virtual bool Start() = 0;
    virtual void Stop() = 0;
    virtual bool SendMessage(const std::string& target_id, const std::vector<uint8_t>& data) = 0;
    // ... 其他接口方法
};
```

### EndpointClient 实现
```cpp
class EndpointClient : public IEndpointService {
public:
    struct ClientConfig : public Config {
        std::string server_address;
        uint16_t server_port;
        bool enable_auto_reconnect;
        // ... 其他客户端特有配置
    };
    
    bool ConnectToServer(const std::string& server_id = "");
    void DisconnectFromServer();
    std::vector<std::string> DiscoverServers(const std::string& server_name = "");
    // ... 其他客户端特有方法
};
```

### EndpointServer 实现
```cpp
class EndpointServer : public IEndpointService {
public:
    struct ServerConfig : public Config {
        uint32_t max_clients;
        bool enable_client_discovery;
        // ... 其他服务器特有配置
    };
    
    bool RegisterClient(const std::string& client_id, const ClientInfo& client_info);
    bool SendToClient(const std::string& client_id, const std::vector<uint8_t>& data);
    void BroadcastToClients(const std::vector<uint8_t>& data);
    // ... 其他服务器特有方法
};
```

### MessageProtocolAdapter 适配器
```cpp
class MessageProtocolAdapter {
public:
    enum class MessageType { Request, Response, Notify, Event, Error };
    
    struct UnifiedMessage {
        MessageType type;
        std::string message_id;
        std::string source_id;
        std::string target_id;
        uint64_t timestamp;
        std::vector<uint8_t> payload;
        nlohmann::json metadata;
    };
    
    bool Initialize(std::shared_ptr<IEndpointService> endpoint_service);
    bool SendMessage(const UnifiedMessage& message);
    UnifiedMessage SendRequest(const UnifiedMessage& request, uint32_t timeout_ms);
    // ... 其他适配器方法
};
```

## 与原有架构的对比

### 原有架构问题
1. **职责混乱**: CommunicationManager 既负责通信又负责业务逻辑
2. **协议耦合**: 消息协议与通信实现紧密耦合
3. **扩展性差**: 难以支持新的协议和传输方式
4. **测试困难**: 组件间依赖复杂，难以单独测试

### 新架构优势
1. **职责清晰**: 每层职责单一，边界明确
2. **协议隔离**: 通信层与协议层完全分离
3. **易于扩展**: 支持插件式协议和传输方式
4. **便于测试**: 各层可独立测试
5. **复用性强**: 接口可被多个实现复用

## 使用示例

### 创建客户端
```cpp
// 1. 创建客户端配置
EndpointClient::ClientConfig config;
config.id = "my_client";

// 2. 创建客户端
auto client = std::make_unique<EndpointClient>(config);

// 3. 注册事件处理器
auto handler = std::make_shared<MyEventHandler>();
client->RegisterEventHandler(handler);

// 4. 初始化和启动
client->Initialize();
client->Start();

// 5. 连接到服务器
client->ConnectToServer("127.0.0.1", 8080);
```

### 创建服务器
```cpp
// 1. 创建服务器配置
EndpointServer::ServerConfig config;
config.id = "my_server";
config.type = EndpointType::Server;
config.port = 8080;
config.max_clients = 100;

// 2. 创建服务器
auto server = std::make_unique<EndpointServer>(config);

// 3. 注册事件处理器
auto handler = std::make_shared<MyServerHandler>();
server->RegisterEventHandler(handler);

// 4. 初始化和启动
server->Initialize();
server->Start();
```

### 服务器事件处理
```cpp
class MyServerHandler : public EndpointServer::EventHandler {
public:
    void OnClientConnected(const std::string& client_id, const ClientInfo& client_info) override {
        std::cout << "客户端连接: " << client_id << std::endl;
    }
    
    void OnClientDisconnected(const std::string& client_id) override {
        std::cout << "客户端断开: " << client_id << std::endl;
    }
    
    void OnMessageReceived(const std::string& client_id, const std::vector<uint8_t>& data) override {
        std::cout << "收到消息: " << client_id << std::endl;
        // 处理消息并发送响应
        std::vector<uint8_t> response = {'O', 'K'};
        g_server->SendToClient(client_id, response);
    }
    
    void OnError(const std::string& client_id, uint16_t error_code, const std::string& error_message) override {
        std::cerr << "错误: " << client_id << " - " << error_message << std::endl;
    }
};
```

### 使用消息协议适配器
```cpp
// 1. 创建适配器配置
MessageProtocolAdapter::Config adapter_config;
adapter_config.protocol_name = "my_protocol";

// 2. 创建适配器
auto adapter = std::make_unique<MessageProtocolAdapter>(adapter_config);

// 3. 注册消息处理器
auto handler = std::make_shared<MyMessageHandler>();
adapter->RegisterMessageHandler(MessageProtocolAdapter::MessageType::Request, handler);

// 4. 初始化适配器
adapter->Initialize(endpoint_service);

// 5. 发送消息
MessageProtocolAdapter::UnifiedMessage message;
message.type = MessageProtocolAdapter::MessageType::Request;
message.message_id = "msg_001";
message.payload = {'H', 'e', 'l', 'l', 'o'};
adapter->SendMessage(message);
```

## 迁移指南

### 从原有架构迁移
1. **替换 CommunicationManager**: 使用 EndpointClient 或 EndpointServer
2. **替换消息处理**: 使用 MessageProtocolAdapter
3. **更新配置**: 使用新的配置结构
4. **重构事件处理**: 使用新的事件处理器接口

### 兼容性考虑
- 新架构保持向后兼容
- 原有代码可以逐步迁移
- 支持混合使用新旧架构

## 示例程序

完整的示例程序位于 `perception_app/runtime/endpoint/examples/` 目录：

### 客户端示例 (`endpoint_client_example.cpp`)
- 演示如何创建和配置端点客户端
- 展示自动发现和连接服务器
- 实现定期消息发送和接收
- 包含完整的事件处理示例

### 服务器示例 (`endpoint_server_example.cpp`)
- 演示如何创建和配置端点服务器
- 展示客户端管理和消息处理
- 实现广播消息功能
- 包含完整的统计信息输出

### 运行示例
```bash
# 编译项目
cd perception_app
mkdir build && cd build
cmake .. && make

# 启动服务器
./endpoint_server_example

# 启动客户端（另一个终端）
./endpoint_client_example
```

详细的使用说明请参考 `examples/README.md`。

## 总结

新的架构设计实现了：
1. **清晰的层次分离**: 通信、端点、协议各层职责明确
2. **协议隔离**: 通信层与协议层完全解耦
3. **易于扩展**: 支持插件式架构
4. **便于维护**: 模块化设计，易于理解和维护
5. **高性能**: 接口设计优化，减少不必要的开销
6. **完整示例**: 提供客户端和服务器完整示例程序

这种设计为未来的功能扩展和协议升级提供了良好的基础。
