# 感知应用推理接口设计

## 概述

本文档描述了感知应用的推理接口设计，该接口实现了推理算法与运行时框架的解耦合，支持本地和分布式推理，为感知应用提供了统一的推理能力。

## 设计目标

### 1. 解耦合设计
- 推理算法与运行时框架完全分离
- 支持插件式推理算法扩展
- 统一的推理接口规范

### 2. 灵活性
- 支持多种推理算法类型
- 支持本地和分布式推理
- 支持实时和批量推理

### 3. 可扩展性
- 易于添加新的推理算法
- 支持算法参数配置
- 支持算法版本管理

## 架构设计

### 核心组件

```
┌─────────────────────────────────────────────────────────────┐
│                    推理接口架构                               │
├─────────────────────────────────────────────────────────────┤
│  Runtime Layer (运行时层)                                    │
│  ├── CameraManager (相机管理器)                              │
│  ├── InferenceManager (推理管理器)                           │
│  └── ResultProcessor (结果处理器)                            │
├─────────────────────────────────────────────────────────────┤
│  Interface Layer (接口层)                                    │
│  ├── InferenceInterface (推理接口)                           │
│  ├── InferenceManager (推理管理器)                           │
│  └── ResultHandler (结果处理器)                              │
├─────────────────────────────────────────────────────────────┤
│  Algorithm Layer (算法层)                                    │
│  ├── LocalInference (本地推理)                               │
│  ├── NetworkInference (网络推理)                             │
│  └── CustomAlgorithms (自定义算法)                           │
├─────────────────────────────────────────────────────────────┤
│  Data Layer (数据层)                                         │
│  ├── FrameSet (帧数据)                                       │
│  ├── InferenceResult (推理结果)                              │
│  └── Configuration (配置管理)                                │
└─────────────────────────────────────────────────────────────┘
```

### 设计原则

1. **接口一致性**: 所有推理算法实现相同的接口
2. **数据标准化**: 统一的输入输出数据格式
3. **配置驱动**: 通过配置文件管理算法参数
4. **错误处理**: 完善的错误处理和恢复机制
5. **性能优化**: 支持异步推理和结果缓存

## 详细设计

### 1. 推理接口 (InferenceInterface)

**核心接口定义:**
```cpp
class InferenceInterface {
public:
    virtual ~InferenceInterface() = default;
    
    /**
     * @brief 初始化推理算法
     * @param config_path 配置文件路径
     * @return 是否初始化成功
     */
    virtual bool Initialize(const std::string& config_path) = 0;
    
    /**
     * @brief 处理帧数据
     * @param frame_set 帧数据集合
     * @return 是否处理成功
     */
    virtual bool Process(const FrameSet& frame_set) = 0;
    
    /**
     * @brief 获取推理结果
     * @return 推理结果字符串
     */
    virtual std::string GetResult() const = 0;
    
    /**
     * @brief 清理资源
     */
    virtual void Cleanup() = 0;
    
    /**
     * @brief 检查是否已初始化
     * @return 是否已初始化
     */
    virtual bool IsInitialized() const = 0;
    
    /**
     * @brief 获取算法名称
     * @return 算法名称
     */
    virtual std::string GetAlgorithmName() const = 0;
};
```

### 2. 推理管理器 (InferenceManager)

**功能特性:**
- 管理多个推理算法实例
- 提供统一的推理接口
- 支持算法切换和组合
- 处理推理结果分发

**主要功能:**
```cpp
class InferenceManager {
public:
    // 单例模式
    static InferenceManager& GetInstance();
    
    // 算法管理
    bool RegisterAlgorithm(const std::string& name, 
                          std::shared_ptr<InferenceInterface> algorithm);
    bool UnregisterAlgorithm(const std::string& name);
    std::shared_ptr<InferenceInterface> GetAlgorithm(const std::string& name);
    
    // 推理处理
    bool ProcessFrame(const FrameSet& frame_set, const std::string& algorithm_name = "");
    std::string GetResult(const std::string& algorithm_name = "") const;
    
    // 状态管理
    bool IsAlgorithmAvailable(const std::string& name) const;
    std::vector<std::string> GetAvailableAlgorithms() const;
};
```

### 3. 帧数据结构 (FrameSet)

**数据结构定义:**
```cpp
struct FrameSet {
    // 彩色图像
    cv::Mat color;
    bool hasColor = false;
    
    // 深度图像
    cv::Mat depthImage;
    bool hasDepth = false;
    
    // 点云数据
    mmind::eye::PointCloud pointCloud;
    bool hasPointCloud = false;
    
    // 时间戳
    uint64_t timestamp = 0;
    
    // 帧ID
    uint32_t frameId = 0;
    
    // 元数据
    std::map<std::string, std::string> metadata;
};
```

### 4. 推理结果格式

**结果数据结构:**
```cpp
struct InferenceResult {
    // 算法名称
    std::string algorithm_name;
    
    // 处理时间
    uint32_t processing_time_ms;
    
    // 置信度
    float confidence;
    
    // 检测结果
    std::vector<Detection> detections;
    
    // 分类结果
    std::vector<Classification> classifications;
    
    // 分割结果
    cv::Mat segmentation_mask;
    
    // 错误信息
    std::string error_message;
    
    // 时间戳
    uint64_t timestamp;
};
```

## 算法实现示例

### 1. 本地推理算法

```cpp
class ExampleInference : public InferenceInterface {
public:
    ExampleInference();
    ~ExampleInference() override;
    
    bool Initialize(const std::string& config_path) override;
    bool Process(const FrameSet& frame_set) override;
    std::string GetResult() const override;
    void Cleanup() override;
    bool IsInitialized() const override;
    std::string GetAlgorithmName() const override;

private:
    // 私有成员
    bool initialized_;
    std::string result_;
    nlohmann::json config_;
    
    // 处理函数
    std::string ProcessColorImage(const cv::Mat& image);
    std::string ProcessDepthImage(const cv::Mat& depth);
    std::string ProcessPointCloud(const mmind::eye::PointCloud& point_cloud);
};
```

### 2. 网络推理算法

```cpp
class NetworkInference : public InferenceInterface {
public:
    NetworkInference();
    ~NetworkInference() override;
    
    bool Initialize(const std::string& config_path) override;
    bool Process(const FrameSet& frame_set) override;
    std::string GetResult() const override;
    void Cleanup() override;
    bool IsInitialized() const override;
    std::string GetAlgorithmName() const override;

private:
    // 通信接口
    std::unique_ptr<CommunicationInterface> comm_interface_;
    
    // 配置
    std::string remote_service_id_;
    uint32_t timeout_ms_;
    
    // 处理函数
    bool SendInferenceRequest(const FrameSet& frame_set);
    bool ReceiveInferenceResponse();
};
```

## 配置管理

### 推理配置文件 (inference_config.json)

```json
{
    "inference": {
        "default_algorithm": "example_inference",
        "algorithms": {
            "example_inference": {
                "type": "local",
                "config_file": "config/example_inference.json",
                "enabled": true,
                "priority": 1
            },
            "network_inference": {
                "type": "network",
                "service_id": "inference_service_001",
                "timeout_ms": 10000,
                "enabled": true,
                "priority": 2
            }
        },
        "processing": {
            "max_concurrent_requests": 5,
            "result_cache_size": 100,
            "enable_async_processing": true
        }
    }
}
```

### 算法配置文件 (example_inference.json)

```json
{
    "algorithm": {
        "name": "Example Inference",
        "version": "1.0.0",
        "description": "示例推理算法"
    },
    "parameters": {
        "confidence_threshold": 0.8,
        "input_size": [640, 480],
        "model_path": "models/example_model.onnx",
        "preprocessing": {
            "normalize": true,
            "mean": [0.485, 0.456, 0.406],
            "std": [0.229, 0.224, 0.225]
        }
    },
    "output": {
        "format": "json",
        "include_confidence": true,
        "include_bounding_boxes": true
    }
}
```

## 使用示例

### 1. 基本使用

```cpp
#include "include/InferenceInterface.hpp"
#include "inference/ExampleInference.hpp"

// 创建推理算法
auto inference = std::make_shared<ExampleInference>();

// 初始化
if (!inference->Initialize("config/example_inference.json")) {
    std::cerr << "推理算法初始化失败" << std::endl;
    return -1;
}

// 处理帧数据
FrameSet frame_set;
frame_set.color = cv::imread("test_image.jpg");
frame_set.hasColor = true;

if (inference->Process(frame_set)) {
    std::string result = inference->GetResult();
    std::cout << "推理结果: " << result << std::endl;
}

// 清理资源
inference->Cleanup();
```

### 2. 使用推理管理器

```cpp
#include "include/InferenceInterface.hpp"
#include "inference/ExampleInference.hpp"

// 获取推理管理器实例
auto& manager = InferenceManager::GetInstance();

// 注册算法
auto algorithm = std::make_shared<ExampleInference>();
manager.RegisterAlgorithm("example", algorithm);

// 处理帧数据
FrameSet frame_set;
frame_set.color = cv::imread("test_image.jpg");
frame_set.hasColor = true;

if (manager.ProcessFrame(frame_set, "example")) {
    std::string result = manager.GetResult("example");
    std::cout << "推理结果: " << result << std::endl;
}
```

### 3. 与相机管理器集成

```cpp
#include "runtime/CameraManager.hpp"
#include "include/InferenceInterface.hpp"

// 创建相机管理器
CameraManager camera_manager;

// 创建推理算法
auto inference = std::make_shared<ExampleInference>();
inference->Initialize("config/example_inference.json");

// 注册推理算法
camera_manager.RegisterInferenceAlgorithm(inference);

// 启用推理
camera_manager.EnableInference();

// 开始相机循环
camera_manager.StartCapture();

// 获取推理结果
std::string result = camera_manager.GetInferenceResult();
```

## 性能优化

### 1. 异步处理
- 支持异步推理处理
- 使用线程池管理推理任务
- 非阻塞结果获取

### 2. 结果缓存
- 缓存推理结果
- 支持结果复用
- 缓存大小可配置

### 3. 批量处理
- 支持批量帧处理
- 优化内存使用
- 提高处理效率

### 4. 算法优化
- 模型量化
- 推理加速
- 内存优化

## 错误处理

### 1. 初始化错误
- 配置文件错误
- 模型加载失败
- 依赖库缺失

### 2. 处理错误
- 输入数据无效
- 推理过程异常
- 内存不足

### 3. 网络错误
- 连接失败
- 超时错误
- 服务不可用

### 4. 错误恢复
- 自动重试机制
- 降级处理
- 错误日志记录

## 扩展指南

### 1. 添加新算法

1. **继承接口类**
```cpp
class MyInference : public InferenceInterface {
    // 实现所有虚函数
};
```

2. **实现核心功能**
```cpp
bool MyInference::Initialize(const std::string& config_path) {
    // 加载配置
    // 初始化模型
    // 设置参数
    return true;
}

bool MyInference::Process(const FrameSet& frame_set) {
    // 预处理数据
    // 执行推理
    // 后处理结果
    return true;
}
```

3. **注册算法**
```cpp
auto algorithm = std::make_shared<MyInference>();
InferenceManager::GetInstance().RegisterAlgorithm("my_algorithm", algorithm);
```

### 2. 自定义结果格式

```cpp
struct MyInferenceResult {
    std::vector<MyDetection> detections;
    float overall_confidence;
    std::string processing_info;
};

// 序列化为JSON
nlohmann::json SerializeResult(const MyInferenceResult& result) {
    nlohmann::json json;
    json["detections"] = result.detections;
    json["confidence"] = result.overall_confidence;
    json["info"] = result.processing_info;
    return json;
}
```

## 总结

推理接口设计为感知应用提供了统一的推理能力，具有以下特点：

1. **解耦合**: 推理算法与运行时框架完全分离
2. **标准化**: 统一的接口和数据格式
3. **可扩展**: 易于添加新的推理算法
4. **高性能**: 支持异步处理和结果缓存
5. **易使用**: 简洁的API和丰富的示例

该设计支持感知应用的灵活部署和扩展，为各种推理需求提供了可靠的解决方案。
