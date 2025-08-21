# 网络部署说明（最终版）

## 拓扑与端口

- 服务发现：UDP 8081
- 设备服务器：TCP 9090
- （可选）第三方模块：示例端口 8080

## Master 节点配置

```cpp
MasterNode::Config config;
config.device_server_id = "device_server_001";
config.device_server_name = "Device Server";
config.device_server_address = "0.0.0.0"; // 监听所有接口
config.device_server_port = 9090;
config.discovery_address = "0.0.0.0";
config.discovery_port = 8081;
config.broadcast_interval = 2000;
config.max_clients = 100;
```

## Client 节点配置

```cpp
ClientNode::Config config;
config.discovery_address = "0.0.0.0";
config.discovery_port = 8081;
config.device_client_id = "device_client_001";
config.device_client_name = "Device Client";
config.controller_client_id = "controller_client_001";
config.controller_client_name = "Controller Client";
config.enable_controller_client = true; // 按需启用
```

## 服务发现地址策略

- 广播 JSON 中若提供有效 address，客户端以其为准
- 仅当地址为空或为 0.0.0.0 时才回退 sender.address()
- 建议跨机器部署时，服务端广播填真实网卡地址

## 验证连通性

```bash
# 监听情况
ss -ltnp | grep 9090

# 本机连通
nc -vz 127.0.0.1 9090

# 跨机连通（替换为真实 IP）
nc -vz <server_ip> 9090
```

## 退出与稳定性

- 监控/心跳/IO 线程带 1s 超时等待，避免长时间阻塞
- Server 启用 SO_REUSEADDR，支持快速重启
