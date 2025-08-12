# 感知应用开发指南

## 概述

本文档为感知应用的开发者提供详细的开发指南，包括环境搭建、代码规范、开发流程、测试方法等内容。

## 开发环境

### 1. 系统要求

- **操作系统**: Ubuntu 20.04 LTS 或更高版本
- **编译器**: GCC 9.0+ 或 Clang 12.0+
- **C++标准**: C++17 或更高版本
- **内存**: 至少 8GB RAM
- **存储**: 至少 10GB 可用空间

### 2. 依赖库

#### 必需依赖
```bash
# 基础开发工具
sudo apt update
sudo apt install -y build-essential cmake git

# ASIO 库
sudo apt install -y libasio-dev

# OpenCV
sudo apt install -y libopencv-dev

# JSON 库
sudo apt install -y nlohmann-json3-dev

# 线程库
sudo apt install -y libpthread-stubs0-dev
```

#### 可选依赖
```bash
# PCL 点云库
sudo apt install -y libpcl-dev

# Halcon 视觉库 (需要许可证)
# 请参考 Halcon 官方安装指南

# CUDA (GPU 加速)
sudo apt install -y nvidia-cuda-toolkit
```

### 3. 环境配置

#### 设置环境变量
```bash
# 添加到 ~/.bashrc
export PERCEPTION_APP_ROOT=/path/to/perception_app
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PERCEPTION_APP_ROOT/lib
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PERCEPTION_APP_ROOT/lib/pkgconfig
```

#### 验证环境
```bash
# 检查编译器
gcc --version
g++ --version

# 检查 CMake
cmake --version

# 检查依赖库
pkg-config --modversion opencv4
pkg-config --modversion asio
```

## 代码规范

### 1. 命名规范

#### 文件命名
- **头文件**: 使用 PascalCase，如 `InferenceInterface.hpp`
- **源文件**: 使用 PascalCase，如 `CameraManager.cpp`
- **配置文件**: 使用 snake_case，如 `inference_config.json`

#### 类命名
```cpp
// 使用 PascalCase
class InferenceInterface;
class CameraManager;
class CommunicationManager;
```

#### 函数命名
```cpp
// 使用 camelCase
bool initializeInference();
void processFrame();
std::string getResult();
```

#### 变量命名
```cpp
// 成员变量使用下划线后缀
class ExampleClass {
private:
    bool initialized_;
    std::string result_;
    int frame_count_;
};

// 局部变量使用 camelCase
void processData() {
    int frameCount = 0;
    std::string resultData;
    bool isProcessing = false;
}
```

#### 常量命名
```cpp
// 使用 UPPER_SNAKE_CASE
const int MAX_FRAME_COUNT = 1000;
const std::string DEFAULT_CONFIG_PATH = "config/default.json";
```

### 2. 代码格式

#### 缩进和空格
```cpp
// 使用 4 个空格缩进
class ExampleClass {
public:
    void exampleFunction() {
        if (condition) {
            doSomething();
        } else {
            doSomethingElse();
        }
    }
};
```

#### 括号风格
```cpp
// 使用 K&R 风格
if (condition) {
    // 代码
} else {
    // 代码
}

for (int i = 0; i < count; ++i) {
    // 代码
}
```

#### 函数参数
```cpp
// 长参数列表换行
bool processFrame(const FrameSet& frame_set,
                 const std::string& algorithm_name,
                 uint32_t timeout_ms);
```

### 3. 注释规范

#### 文件头注释
```cpp
/**
 * @file CameraManager.hpp
 * @brief 相机管理器头文件
 * @author Your Name
 * @date 2024-01-01
 * @version 1.0.0
 */
```

#### 类注释
```cpp
/**
 * @brief 相机管理器类
 * 
 * 负责相机的初始化、配置、数据采集等功能。
 * 支持多种相机类型和配置参数。
 * 
 * @example
 * ```cpp
 * CameraManager manager;
 * manager.Initialize("config/camera.json");
 * manager.StartCapture();
 * ```
 */
class CameraManager {
    // 类实现
};
```

#### 函数注释
```cpp
/**
 * @brief 初始化相机管理器
 * 
 * 根据配置文件初始化相机，设置参数并建立连接。
 * 
 * @param config_path 配置文件路径
 * @return 是否初始化成功
 * @throws std::runtime_error 配置文件不存在或格式错误
 * 
 * @note 此函数必须在 StartCapture() 之前调用
 */
bool Initialize(const std::string& config_path);
```

#### 行内注释
```cpp
// 检查相机状态
if (!camera_.isConnected()) {
    return false;  // 相机未连接，返回失败
}

// 处理帧数据
auto frame = camera_.captureFrame();  // 获取当前帧
```

## 开发流程

### 1. 项目结构

```
perception_app/
├── doc/                    # 文档
├── include/                # 公共头文件
├── communication/          # 通信层
├── message/                # 消息层
├── runtime/                # 运行时框架
├── inference/              # 推理算法
├── config/                 # 配置文件
├── examples/               # 示例代码
├── tests/                  # 测试代码
├── scripts/                # 构建脚本
└── third_party/            # 第三方库
```

### 2. 开发步骤

#### 步骤1: 创建功能分支
```bash
# 从主分支创建功能分支
git checkout -b feature/new-feature

# 或从开发分支创建
git checkout -b feature/new-feature develop
```

#### 步骤2: 实现功能
```bash
# 创建新文件
touch include/NewInterface.hpp
touch runtime/NewManager.cpp

# 编辑文件
vim include/NewInterface.hpp
vim runtime/NewManager.cpp
```

#### 步骤3: 添加测试
```bash
# 创建测试文件
touch tests/test_new_feature.cpp

# 编写测试用例
vim tests/test_new_feature.cpp
```

#### 步骤4: 编译和测试
```bash
# 编译项目
./build.sh

# 运行测试
./build/tests/test_new_feature

# 运行所有测试
ctest --output-on-failure
```

#### 步骤5: 代码审查
```bash
# 提交代码
git add .
git commit -m "feat: add new feature implementation"

# 推送到远程分支
git push origin feature/new-feature

# 创建 Pull Request
```

### 3. 提交规范

#### 提交消息格式
```
<type>(<scope>): <subject>

<body>

<footer>
```

#### 类型说明
- **feat**: 新功能
- **fix**: 修复bug
- **docs**: 文档更新
- **style**: 代码格式调整
- **refactor**: 代码重构
- **test**: 测试相关
- **chore**: 构建过程或辅助工具的变动

#### 示例
```bash
# 新功能
git commit -m "feat(inference): add new inference algorithm"

# 修复bug
git commit -m "fix(camera): fix camera initialization issue"

# 文档更新
git commit -m "docs: update API documentation"
```

## 测试指南

### 1. 单元测试

#### 测试框架
使用 Google Test 框架进行单元测试。

#### 测试文件结构
```cpp
// tests/test_camera_manager.cpp
#include <gtest/gtest.h>
#include "runtime/CameraManager.hpp"

class CameraManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的设置
    }
    
    void TearDown() override {
        // 测试后的清理
    }
    
    CameraManager manager_;
};

TEST_F(CameraManagerTest, InitializationTest) {
    // 测试初始化功能
    EXPECT_TRUE(manager_.Initialize("config/test_camera.json"));
    EXPECT_TRUE(manager_.IsInitialized());
}

TEST_F(CameraManagerTest, CaptureTest) {
    // 测试数据采集功能
    manager_.Initialize("config/test_camera.json");
    auto frame = manager_.CaptureFrame();
    EXPECT_FALSE(frame.color.empty());
}
```

#### 运行测试
```bash
# 编译测试
cmake --build build --target test_camera_manager

# 运行测试
./build/tests/test_camera_manager

# 运行所有测试
ctest --output-on-failure
```

### 2. 集成测试

#### 测试场景
```cpp
// tests/integration_test.cpp
#include <gtest/gtest.h>
#include "runtime/CameraManager.hpp"
#include "inference/ExampleInference.hpp"

class IntegrationTest : public ::testing::Test {
protected:
    CameraManager camera_manager_;
    std::shared_ptr<ExampleInference> inference_;
};

TEST_F(IntegrationTest, CameraInferenceIntegration) {
    // 初始化相机
    ASSERT_TRUE(camera_manager_.Initialize("config/test_camera.json"));
    
    // 初始化推理算法
    inference_ = std::make_shared<ExampleInference>();
    ASSERT_TRUE(inference_->Initialize("config/test_inference.json"));
    
    // 注册推理算法
    camera_manager_.RegisterInferenceAlgorithm(inference_);
    
    // 启用推理
    camera_manager_.EnableInference();
    
    // 开始采集
    camera_manager_.StartCapture();
    
    // 等待处理
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 获取结果
    std::string result = camera_manager_.GetInferenceResult();
    EXPECT_FALSE(result.empty());
}
```

### 3. 性能测试

#### 基准测试
```cpp
// tests/benchmark_test.cpp
#include <benchmark/benchmark.h>
#include "inference/ExampleInference.hpp"

static void BM_InferenceProcessing(benchmark::State& state) {
    ExampleInference inference;
    inference.Initialize("config/benchmark_inference.json");
    
    FrameSet frame_set;
    frame_set.color = cv::Mat::zeros(640, 480, CV_8UC3);
    frame_set.hasColor = true;
    
    for (auto _ : state) {
        inference.Process(frame_set);
    }
}

BENCHMARK(BM_InferenceProcessing);

BENCHMARK_MAIN();
```

#### 运行基准测试
```bash
# 编译基准测试
cmake --build build --target benchmark_test

# 运行基准测试
./build/tests/benchmark_test
```

## 调试指南

### 1. 日志系统

#### 日志级别
```cpp
#include "include/Logger.hpp"

// 设置日志级别
Logger::SetLevel(LogLevel::DEBUG);

// 输出日志
LOG_DEBUG("Debug message: {}", value);
LOG_INFO("Info message: {}", value);
LOG_WARN("Warning message: {}", value);
LOG_ERROR("Error message: {}", value);
```

#### 日志配置
```json
{
    "logging": {
        "level": "INFO",
        "file": "logs/perception_app.log",
        "max_size": "10MB",
        "max_files": 5,
        "format": "[{timestamp}] [{level}] [{file}:{line}] {message}"
    }
}
```

### 2. 调试工具

#### GDB 调试
```bash
# 编译调试版本
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 启动调试
gdb ./build/perception_app

# 设置断点
(gdb) break CameraManager::Initialize
(gdb) break InferenceInterface::Process

# 运行程序
(gdb) run

# 查看变量
(gdb) print config_path
(gdb) print initialized_
```

#### Valgrind 内存检查
```bash
# 检查内存泄漏
valgrind --leak-check=full --show-leak-kinds=all ./build/perception_app

# 检查内存错误
valgrind --tool=memcheck ./build/perception_app
```

### 3. 性能分析

#### 使用 gprof
```bash
# 编译时添加性能分析
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-pg" ..

# 运行程序
./build/perception_app

# 分析性能
gprof ./build/perception_app gmon.out > analysis.txt
```

#### 使用 perf
```bash
# 性能分析
perf record ./build/perception_app
perf report

# 热点分析
perf top -p $(pgrep perception_app)
```

## 部署指南

### 1. 构建发布版本

```bash
# 创建发布构建
mkdir build_release
cd build_release

# 配置发布版本
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DENABLE_TESTS=OFF \
      -DENABLE_EXAMPLES=OFF \
      ..

# 编译
make -j$(nproc)

# 安装
sudo make install
```

### 2. 打包

#### 创建安装包
```bash
# 创建 DEB 包
cpack -G DEB

# 创建 RPM 包
cpack -G RPM

# 创建 TGZ 包
cpack -G TGZ
```

#### Docker 镜像
```dockerfile
# Dockerfile
FROM ubuntu:20.04

# 安装依赖
RUN apt-get update && apt-get install -y \
    libasio-dev \
    libopencv-dev \
    nlohmann-json3-dev

# 复制应用程序
COPY perception_app /usr/local/bin/
COPY config/ /usr/local/etc/perception_app/

# 设置工作目录
WORKDIR /usr/local/etc/perception_app

# 运行应用
CMD ["perception_app"]
```

### 3. 部署配置

#### 系统服务
```ini
# /etc/systemd/system/perception-app.service
[Unit]
Description=Perception Application
After=network.target

[Service]
Type=simple
User=perception
ExecStart=/usr/local/bin/perception_app
WorkingDirectory=/usr/local/etc/perception_app
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

#### 启动服务
```bash
# 启用服务
sudo systemctl enable perception-app

# 启动服务
sudo systemctl start perception-app

# 查看状态
sudo systemctl status perception-app

# 查看日志
sudo journalctl -u perception-app -f
```

## 常见问题

### 1. 编译问题

#### 依赖库找不到
```bash
# 设置 PKG_CONFIG_PATH
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig

# 或使用 cmake 指定路径
cmake -DOpenCV_DIR=/usr/local/lib/cmake/opencv4 ..
```

#### 链接错误
```bash
# 检查库文件
ls -la /usr/local/lib/libperception*

# 更新动态库缓存
sudo ldconfig
```

### 2. 运行时问题

#### 权限问题
```bash
# 检查文件权限
ls -la /dev/video*

# 添加用户到视频组
sudo usermod -a -G video $USER
```

#### 配置文件问题
```bash
# 检查配置文件语法
python3 -m json.tool config/inference_config.json

# 验证配置文件
./perception_app --validate-config
```

### 3. 性能问题

#### 内存泄漏
```bash
# 使用 valgrind 检查
valgrind --tool=memcheck --leak-check=full ./perception_app
```

#### CPU 使用率高
```bash
# 使用 top 监控
top -p $(pgrep perception_app)

# 使用 perf 分析
perf top -p $(pgrep perception_app)
```

## 贡献指南

### 1. 代码贡献

1. Fork 项目
2. 创建功能分支
3. 实现功能
4. 添加测试
5. 提交代码
6. 创建 Pull Request

### 2. 文档贡献

1. 更新相关文档
2. 添加示例代码
3. 修正错误
4. 改进说明

### 3. 问题报告

1. 使用 Issue 模板
2. 提供详细描述
3. 包含复现步骤
4. 附加日志信息

## 总结

本开发指南为感知应用的开发者提供了完整的开发流程和最佳实践。遵循这些规范可以确保代码质量、提高开发效率，并为项目的长期维护奠定基础。

如有问题或建议，请通过以下方式联系：
- 创建 Issue
- 发送邮件
- 参与讨论
