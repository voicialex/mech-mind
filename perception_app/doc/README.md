# Perception Application

## 项目概述

Perception Application 是一个基于ASIO的分布式感知应用框架，提供独立的通信层和消息层，支持感知应用的网络化部署和扩展。

## 项目结构

```
perception_app/
├── doc/                           # 项目文档
│   ├── README.md                  # 项目主文档
│   ├── CommunicationArchitecture.md # 通信架构（最终版）
│   ├── NetworkDeployment.md       # 网络部署说明（最终版）
│   ├── InferenceInterface.md      # 推理接口设计
│   └── DevelopmentGuide.md        # 开发指南
├── include/                       # 公共头文件
│   ├── InferenceInterface.hpp     # 推理接口定义
│   ├── Logger.hpp                 # 日志系统
│   └── FrameSet.hpp               # 帧数据结构
├── communication/                 # 通信层
│   ├── CommunicationManager.hpp   # 通信管理器
│   └── CommunicationInterface.hpp # 通信接口
├── message/                       # 消息层
│   ├── MessageProtocol.hpp        # 消息协议
│   └── PerceptionMessages.hpp     # 感知消息定义
├── runtime/                       # 运行时框架
│   ├── CameraManager.hpp          # 相机管理器
│   ├── CameraManager.cpp          # 相机管理器实现
│   ├── InferenceManager.cpp       # 推理管理器
│   └── main.cpp                   # 主程序
├── inference/                     # 推理算法
│   ├── ExampleInference.hpp       # 示例推理算法
│   └── ExampleInference.cpp       # 示例推理算法实现
├── config/                        # 配置文件
│   ├── inference_config.json      # 推理配置
│   └── communication_config.json  # 通信配置
├── examples/                      # 示例代码
│   └── NetworkInferenceExample.cpp # 通信接口使用示例
├── CMakeLists.txt                 # CMake构建配置
└── build.sh                       # 构建脚本
```

## 核心特性

### 1. 独立通信层
- 基于ASIO的高性能异步通信
- 服务发现和自动连接管理
- 支持TCP/UDP传输协议
- 自动错误恢复和重连机制

### 2. 标准化消息协议
- 符合充电枪协议规范
- 支持CRC16校验和错误检测
- 消息序列化和反序列化
- 灵活的消息路由机制

### 3. 模块化推理接口
- 解耦合的推理算法设计
- 支持本地和分布式推理
- 统一的推理结果格式
- 可扩展的算法框架

### 4. 运行时框架
- 相机数据采集和管理
- 推理结果处理和分发
- 配置管理和日志系统
- 异常处理和错误恢复

## 快速开始

### 1. 环境要求
- C++17 或更高版本
- CMake 3.16 或更高版本
- ASIO 库
- OpenCV 4.x
- nlohmann/json

### 2. 编译构建
```bash
# 克隆项目
git clone <repository_url>
cd perception_app

# 使用构建脚本
./build.sh

# 或使用CMake
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3. 运行示例
```bash
# 运行主程序
./perception_app

# 运行通信示例
./examples/NetworkInferenceExample
```

## 使用指南

### 1. 创建推理算法
```cpp
#include "include/InferenceInterface.hpp"

class MyInference : public InferenceInterface {
public:
    bool Initialize(const std::string& config_path) override;
    bool Process(const FrameSet& frame_set) override;
    std::string GetResult() const override;
    void Cleanup() override;
    bool IsInitialized() const override;
    std::string GetAlgorithmName() const override;
};
```

### 2. 使用通信接口
```cpp
#include "communication/CommunicationInterface.hpp"
#include "message/PerceptionMessages.hpp"

// 创建通信接口
CommunicationInterface::Config config;
config.local_service_id = "my_service";
auto comm_interface = std::make_unique<CommunicationInterface>(config);

// 初始化并启动
comm_interface->Initialize();
comm_interface->Start();

// 发送消息
auto message = std::make_shared<PerceptionStartMessage>();
comm_interface->SendMessage("target_service", message);
```

### 3. 配置管理
```json
{
    "communication": {
        "local_service": {
            "service_id": "perception_app_001",
            "service_name": "Perception Application"
        },
        "network": {
            "local_port": 8080,
            "discovery_port": 8081
        }
    }
}
```

## 开发指南

详细的开发指南请参考：
- [通信架构设计](CommunicationArchitecture.md)
- [推理接口设计](InferenceInterface.md)
- [开发指南](DevelopmentGuide.md)

## 贡献指南

1. Fork 项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开 Pull Request

## 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 联系方式

- 项目维护者: [Your Name]
- 邮箱: [your.email@example.com]
- 项目链接: [https://github.com/your-username/perception_app]
