#include "../communication/CommunicationInterface.hpp"
#include "../message/PerceptionMessages.hpp"
#include "../include/Logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace perception;

/**
 * @brief 通信接口使用示例
 *
 * 演示如何使用CommunicationInterface进行消息通信
 */
class CommunicationExample {
 public:
  CommunicationExample() {
    // 配置通信接口
    CommunicationInterface::Config config;
    config.local_service_id = "example_client_001";
    config.local_service_name = "Example Client";
    config.local_port = 8082;
    config.discovery_port = 8083;
    config.heartbeat_interval = 5000;
    config.enable_auto_reconnect = true;

    comm_interface_ = std::make_unique<CommunicationInterface>(config);

    // 设置回调函数
    comm_interface_->RegisterMessageCallback([this](const Message::Ptr &message) { OnMessageReceived(message); });

    comm_interface_->RegisterConnectionCallback([this](const std::string &service_id, bool connected) { OnConnectionStatus(service_id, connected); });

    comm_interface_->RegisterErrorCallback([this](const std::string &service_id, uint16_t error_code) { OnError(service_id, error_code); });
  }

  /**
   * @brief 运行示例
   */
  void Run() {
    std::cout << "=== 通信接口示例 ===" << std::endl;

    // 1. 初始化通信接口
    if (!Initialize()) {
      std::cout << "初始化失败！" << std::endl;
      return;
    }

    // 2. 发现并连接服务
    if (!ConnectToServices()) {
      std::cout << "连接服务失败！" << std::endl;
      return;
    }

    // 3. 执行消息通信测试
    RunCommunicationTest();

    // 4. 清理资源
    Cleanup();
  }

 private:
  /**
   * @brief 初始化
   */
  bool Initialize() {
    std::cout << "正在初始化通信接口..." << std::endl;

    if (!comm_interface_->Initialize()) {
      std::cout << "通信接口初始化失败" << std::endl;
      return false;
    }

    if (!comm_interface_->Start()) {
      std::cout << "通信接口启动失败" << std::endl;
      return false;
    }

    std::cout << "通信接口初始化成功" << std::endl;
    return true;
  }

  /**
   * @brief 连接服务
   */
  bool ConnectToServices() {
    std::cout << "正在发现服务..." << std::endl;

    // 等待服务发现
    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto services = comm_interface_->DiscoverServices();
    if (services.empty()) {
      std::cout << "未发现可用的服务" << std::endl;
      return false;
    }

    std::cout << "发现 " << services.size() << " 个服务:" << std::endl;
    for (const auto &service : services) {
      std::cout << "  - " << service.service_name << " (" << service.service_id << ") at " << service.address << ":" << service.port << std::endl;
    }

    // 连接到第一个可用服务
    std::string service_id = services[0].service_id;
    std::cout << "正在连接到服务: " << service_id << std::endl;

    if (!comm_interface_->ConnectToService(service_id)) {
      std::cout << "连接服务失败" << std::endl;
      return false;
    }

    std::cout << "成功连接到服务: " << service_id << std::endl;
    return true;
  }

  /**
   * @brief 运行通信测试
   */
  void RunCommunicationTest() {
    std::cout << "开始通信测试..." << std::endl;

    // 测试1: 发送感知启动消息
    std::cout << "\n--- 测试1: 发送感知启动消息 ---" << std::endl;
    auto start_message = std::make_shared<PerceptionStartMessage>();
    nlohmann::json config;
    config["model_name"] = "example_model";
    config["confidence_threshold"] = 0.8f;
    config["input_size"] = {{640, 480}};
    start_message->SetPerceptionConfig(config);

    if (comm_interface_->SendMessage("perception_service", start_message)) {
      std::cout << "感知启动消息发送成功" << std::endl;
    } else {
      std::cout << "感知启动消息发送失败" << std::endl;
    }

    // 测试2: 发送推理请求
    std::cout << "\n--- 测试2: 发送推理请求 ---" << std::endl;
    auto inference_message = std::make_shared<InferenceRequestMessage>();
    inference_message->SetInferenceParams("test_model", 0.8f, cv::Size(640, 480));

    // 创建测试图像
    cv::Mat test_image = cv::Mat::zeros(480, 640, CV_8UC3);
    cv::rectangle(test_image, cv::Point(100, 100), cv::Point(300, 300), cv::Scalar(0, 255, 0), 2);
    inference_message->SetImageData(test_image);

    if (comm_interface_->SendMessage("inference_service", inference_message)) {
      std::cout << "推理请求消息发送成功" << std::endl;
    } else {
      std::cout << "推理请求消息发送失败" << std::endl;
    }

    // 测试3: 发送请求并等待响应
    std::cout << "\n--- 测试3: 发送请求并等待响应 ---" << std::endl;
    auto status_message = std::make_shared<PerceptionStatusMessage>();
    status_message->SetPerceptionStatus({{"status", "running"}});

    try {
      auto response = comm_interface_->SendRequest("perception_service", status_message, 5000);
      if (response) {
        std::cout << "收到响应: " << response->ToString() << std::endl;
      } else {
        std::cout << "未收到响应" << std::endl;
      }
    } catch (const std::exception &e) {
      std::cout << "请求超时: " << e.what() << std::endl;
    }

    // 测试4: 广播消息
    std::cout << "\n--- 测试4: 广播消息 ---" << std::endl;
    auto broadcast_message = std::make_shared<SystemStatusMessage>();
    broadcast_message->SetSystemStatus({{"cpu_usage", 45.5}, {"memory_usage", 60.2}});

    comm_interface_->BroadcastMessage(broadcast_message, "perception");
    std::cout << "广播消息发送完成" << std::endl;

    // 等待一段时间接收消息
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }

  /**
   * @brief 清理资源
   */
  void Cleanup() {
    std::cout << "正在清理资源..." << std::endl;

    comm_interface_->Stop();
    comm_interface_->Cleanup();

    std::cout << "资源清理完成" << std::endl;
  }

  /**
   * @brief 消息接收回调
   */
  void OnMessageReceived(const Message::Ptr &message) { std::cout << "收到消息 [" << message->GetMessageId() << "]: " << message->ToString() << std::endl; }

  /**
   * @brief 连接状态回调
   */
  void OnConnectionStatus(const std::string &service_id, bool connected) { std::cout << "连接状态 [" << service_id << "]: " << (connected ? "已连接" : "已断开") << std::endl; }

  /**
   * @brief 错误回调
   */
  void OnError(const std::string &service_id, uint16_t error_code) { std::cout << "错误 [" << service_id << "]: 0x" << std::hex << error_code << std::dec << std::endl; }

 private:
  std::unique_ptr<CommunicationInterface> comm_interface_;
};

/**
 * @brief 主函数
 */
int main() {
  try {
    CommunicationExample example;
    example.Run();
  } catch (const std::exception &e) {
    std::cerr << "示例运行失败: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
