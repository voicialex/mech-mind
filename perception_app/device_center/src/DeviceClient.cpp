#include "DeviceClient.hpp"
#include "communication/CommunicationInterface.hpp"
#include "message/PerceptionMessages.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace device_center;

namespace {
uint64_t now_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
}  // namespace

DeviceClient::DeviceClient(const Config &config) : config_(config) {}
DeviceClient::DeviceClient() : config_() {}  // 默认构造函数使用默认配置
DeviceClient::~DeviceClient() { Stop(); }

// 实现通信删除器
void DeviceClient::CommunicationDeleter::operator()(perception::CommunicationInterface* ptr) const {
  if (ptr) {
    delete ptr;
  }
}

bool DeviceClient::Initialize(const std::string &config_path) {
  if (initialized_) return true;
  LoadConfig(config_path);
  InitializeCommunication();
  client_start_time_ = now_ms();
  initialized_ = true;
  return true;
}

bool DeviceClient::ConnectToServer(const std::string & /*server_id*/) {
  if (!initialized_) return false;
  
  // 使用ASIO通信层连接服务器
  try {
    if (comm_interface_) {
      // 发现服务器
      auto services = comm_interface_->DiscoverServices("Device Management Server");
      if (!services.empty()) {
        // 连接到第一个发现的服务器
        const auto& server = services[0];
        if (comm_interface_->ConnectToService(server.service_id)) {
          connected_ = true;
          connected_server_id_ = server.service_id;
          std::cout << "成功连接到服务器: " << server.service_id << std::endl;
          return true;
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "连接服务器异常: " << e.what() << std::endl;
  }
  
  return false;
}

void DeviceClient::DisconnectFromServer() { 
  if (comm_interface_) {
    comm_interface_->DisconnectFromService(connected_server_id_);
  }
  connected_ = false; 
}

bool DeviceClient::Start() {
  if (!initialized_) return false;
  if (running_) return true;
  running_ = true;
  StartHeartbeat();
  return true;
}

void DeviceClient::Stop() {
  if (!running_) return;
  running_ = false;
  StopHeartbeat();
}

void DeviceClient::Run() {
  // 简化：不启动额外循环
}

void DeviceClient::Cleanup() {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  registered_devices_.clear();
  device_status_.clear();
  device_configs_.clear();
  device_heartbeat_enabled_.clear();
}

bool DeviceClient::RegisterDevice(const std::string &device_id, const nlohmann::json &config) {
  // 先本地注册
  DeviceConfig dc;
  dc.device_id = device_id;
  dc.device_name = device_id;
  dc.config = config;
  bool local_success = RegisterDevice(dc);

  // 如果已连接到服务器，向服务器注册设备
  if (connected_ && local_success) {
    DeviceRegisterRequest request;
    request.device_id = device_id;
    request.device_name = device_id;
    request.client_id = config_.service_id;
    request.capabilities = {DeviceCapability::Capture, DeviceCapability::Configure, DeviceCapability::Status};

    // 通过通信接口发送注册请求
    if (comm_interface_) {
      try {
        // 创建注册消息
        auto message = std::make_shared<perception::Message>();
        message->SetMessageId(perception::MessageIds::PERCEPTION_START);
        // 简化实现：直接发送请求数据
        std::vector<uint8_t> request_data;
        // 这里应该序列化request，简化实现
        
        if (comm_interface_->SendMessage(connected_server_id_, message)) {
          std::cout << "向服务器注册设备: " << device_id << std::endl;
          return true;
        }
      } catch (const std::exception& e) {
        std::cerr << "注册设备到服务器失败: " << e.what() << std::endl;
      }
    }
  }

  return local_success;
}

bool DeviceClient::RegisterDevice(const DeviceConfig &device_config) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  DeviceInfo info{};
  info.device_id = device_config.device_id;
  info.device_name = device_config.device_name;

  info.device_model = device_config.device_model;
  info.device_version = device_config.device_version;
  info.status = DeviceStatus::Online;
  info.status_message = "registered";
  info.last_heartbeat = now_ms();

  registered_devices_[info.device_id] = info;
  device_configs_[info.device_id] = device_config;
  ++total_devices_registered_;
  return true;
}

bool DeviceClient::UnregisterDevice(const std::string &device_id) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  auto it = registered_devices_.find(device_id);
  if (it == registered_devices_.end()) return false;
  registered_devices_.erase(it);
  device_status_.erase(device_id);
  device_configs_.erase(device_id);
  device_heartbeat_enabled_.erase(device_id);
  return true;
}

bool DeviceClient::ReportDeviceStatus(const std::string &device_id, const DeviceStatusInfo &status) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  device_status_[device_id] = status;
  ++total_status_reports_;
  
  // 如果已连接服务器，上报状态
  if (connected_ && comm_interface_) {
    try {
      auto message = std::make_shared<perception::Message>();
      message->SetMessageId(perception::MessageIds::PERCEPTION_STATUS);
              // 简化实现：直接发送状态数据
        std::vector<uint8_t> status_data;
        // 这里应该序列化status，简化实现
      
      comm_interface_->SendMessage(connected_server_id_, message);
    } catch (const std::exception& e) {
      std::cerr << "上报设备状态失败: " << e.what() << std::endl;
    }
  }
  
  return true;
}

bool DeviceClient::ReportDeviceData(const DeviceData & /*device_data*/) {
  ++total_data_reports_;
  return true;
}

bool DeviceClient::RespondToCommand(const std::string & /*device_id*/, const std::string & /*command_id*/, const DeviceResponse & /*response*/) {
  ++total_commands_processed_;
  return true;
}

bool DeviceClient::RegisterDeviceHandler(const std::string &device_id, DeviceHandlerPtr handler) {
  std::lock_guard<std::mutex> lk(handlers_mutex_);
  device_handlers_[device_id] = std::move(handler);
  return true;
}

void DeviceClient::RegisterEventHandler(EventHandlerPtr handler) {
  std::lock_guard<std::mutex> lk(handlers_mutex_);
  event_handler_ = std::move(handler);
}

bool DeviceClient::RegisterCommandCallback(const std::string &device_id, CommandCallback callback) {
  std::lock_guard<std::mutex> lk(handlers_mutex_);
  command_callbacks_[device_id] = std::move(callback);
  return true;
}

bool DeviceClient::RegisterStatusCallback(const std::string &device_id, StatusCallback callback) {
  std::lock_guard<std::mutex> lk(handlers_mutex_);
  status_callbacks_[device_id] = std::move(callback);
  return true;
}

std::vector<std::string> DeviceClient::DiscoverServers(const std::string & /*server_name*/) {
  // 使用ASIO通信层发现服务器
  if (comm_interface_) {
    try {
      auto services = comm_interface_->DiscoverServices("Device Management Server");
      std::vector<std::string> server_list;
      for (const auto& service : services) {
        server_list.push_back(service.service_id);
      }
      return server_list;
    } catch (const std::exception& e) {
      std::cerr << "发现服务器失败: " << e.what() << std::endl;
    }
  }
  return {};
}

std::vector<DeviceInfo> DeviceClient::GetRegisteredDevices() const {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  std::vector<DeviceInfo> v;
  v.reserve(registered_devices_.size());
  for (const auto &kv : registered_devices_) v.push_back(kv.second);
  return v;
}

DeviceInfo DeviceClient::GetDeviceInfo(const std::string &device_id) const {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  auto it = registered_devices_.find(device_id);
  if (it != registered_devices_.end()) return it->second;
  return DeviceInfo{};
}

DeviceStatusInfo DeviceClient::GetDeviceStatus(const std::string &device_id) const {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  auto it = device_status_.find(device_id);
  if (it != device_status_.end()) return it->second;
  DeviceStatusInfo s{};
  s.device_id = device_id;
  s.status = DeviceStatus::Offline;
  s.timestamp = now_ms();
  return s;
}

bool DeviceClient::IsDeviceRegistered(const std::string &device_id) const {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  return registered_devices_.count(device_id) > 0;
}

bool DeviceClient::IsConnectedToServer() const { return connected_; }

nlohmann::json DeviceClient::GetConnectionStatus() const {
  nlohmann::json status_json;
  status_json["connected"] = connected_.load();
  status_json["server_id"] = connected_server_id_;
  return status_json;
}

nlohmann::json DeviceClient::GetClientStatistics() const {
  nlohmann::json stats_json;
  stats_json["total_devices_registered"] = total_devices_registered_;
  stats_json["total_status_reports"] = total_status_reports_;
  stats_json["total_data_reports"] = total_data_reports_;
  stats_json["total_commands_processed"] = total_commands_processed_;
  stats_json["total_errors"] = total_errors_;
  stats_json["uptime_ms"] = (client_start_time_ == 0) ? 0 : (now_ms() - client_start_time_);
  return stats_json;
}

DeviceClient::Config DeviceClient::GetConfig() const { return config_; }

bool DeviceClient::IsRunning() const { return running_; }

bool DeviceClient::IsInitialized() const { return initialized_; }

bool DeviceClient::SetDeviceConfig(const std::string &device_id, const nlohmann::json &config) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  auto it = device_configs_.find(device_id);
  if (it == device_configs_.end()) return false;
  it->second.config = config;
  return true;
}

nlohmann::json DeviceClient::GetDeviceConfig(const std::string &device_id) const {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  auto it = device_configs_.find(device_id);
  if (it != device_configs_.end()) return it->second.config;
  return nlohmann::json::object();
}

bool DeviceClient::EnableDeviceHeartbeat(const std::string &device_id, uint32_t interval_ms) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  device_heartbeat_enabled_[device_id] = true;
  (void)interval_ms;
  return true;
}

bool DeviceClient::DisableDeviceHeartbeat(const std::string &device_id) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  device_heartbeat_enabled_[device_id] = false;
  return true;
}

void DeviceClient::LoadConfig(const std::string &config_path) {
  if (config_path.empty()) return;
  try {
    std::ifstream ifs(config_path);
    if (!ifs.is_open()) return;
    nlohmann::json config_json;
    ifs >> config_json;
    if (config_json.contains("client")) {
      const auto &client_config = config_json["client"];
      if (client_config.contains("service_id")) config_.service_id = client_config["service_id"].get<std::string>();
      if (client_config.contains("service_name")) config_.service_name = client_config["service_name"].get<std::string>();
      if (client_config.contains("server_address")) config_.server_address = client_config["server_address"].get<std::string>();
      if (client_config.contains("server_port")) config_.server_port = client_config["server_port"].get<uint16_t>();
      if (client_config.contains("local_port")) config_.local_port = client_config["local_port"].get<uint16_t>();
    }
    
    // 从communication.network部分读取discovery_port
    if (config_json.contains("communication") && 
        config_json["communication"].contains("network")) {
      const auto &network_config = config_json["communication"]["network"];
      if (network_config.contains("discovery_port")) {
        config_.discovery_port = network_config["discovery_port"].get<uint16_t>();
      }
    }
    
    std::cout << "[CONFIG] 客户端配置加载完成:" << std::endl;
    std::cout << "  service_id: " << config_.service_id << std::endl;
    std::cout << "  local_port: " << config_.local_port << std::endl;
    std::cout << "  discovery_port: " << config_.discovery_port << std::endl;
    std::cout << "  server_port: " << config_.server_port << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "加载配置文件失败: " << e.what() << std::endl;
  }
}

void DeviceClient::InitializeCommunication() {
  // 使用ASIO通信层
  try {
    perception::CommunicationInterface::Config comm_config;
    comm_config.local_service_id = config_.service_id;
    comm_config.local_service_name = config_.service_name;
    comm_config.local_address = "0.0.0.0";
    comm_config.local_port = config_.local_port;
    comm_config.discovery_port = config_.discovery_port;
    comm_config.is_server = false; // 客户端模式
    
    auto comm_interface = new perception::CommunicationInterface(comm_config);
    
    // 注册消息回调
    comm_interface->RegisterMessageCallback([this](const std::shared_ptr<perception::Message>& message) {
      HandleServerMessage(message);
    });
    
    comm_interface->RegisterConnectionCallback([this](const std::string& service_id, bool connected) {
      HandleServerConnection(service_id, connected);
    });
    
    comm_interface->RegisterErrorCallback([this](const std::string& service_id, uint16_t error_code) {
      HandleCommunicationError(service_id, error_code);
    });
    
          if (comm_interface->Initialize() && comm_interface->Start()) {
        comm_interface_.reset(comm_interface);
        std::cout << "通信接口初始化成功" << std::endl;
      } else {
        delete comm_interface;
        std::cerr << "通信接口初始化失败" << std::endl;
      }
  } catch (const std::exception& e) {
    std::cerr << "初始化通信接口异常: " << e.what() << std::endl;
  }
}

void DeviceClient::StartHeartbeat() {
  if (heartbeat_running_) return;
  heartbeat_running_ = true;
  heartbeat_thread_ = std::thread([this]() {
    while (running_) {
      ProcessHeartbeat();
      std::this_thread::sleep_for(std::chrono::milliseconds(config_.heartbeat_interval));
    }
    heartbeat_running_ = false;
  });
}

void DeviceClient::StopHeartbeat() {
  if (heartbeat_thread_.joinable()) heartbeat_thread_.join();
}

void DeviceClient::ProcessHeartbeat() {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  const uint64_t t = now_ms();
  for (auto &kv : registered_devices_) {
    auto &info = kv.second;
    info.last_heartbeat = t;
    if (device_heartbeat_enabled_[info.device_id]) {
      DeviceStatusInfo s{};
      s.device_id = info.device_id;
      s.status = DeviceStatus::Online;
      s.status_message = "heartbeat";
      s.timestamp = t;
      device_status_[info.device_id] = s;
    }
  }
}

void DeviceClient::HandleDeviceRegistration(const DeviceRegisterRequest &) {}
void DeviceClient::HandleDeviceDiscovery(const DeviceDiscoveryRequest &) {}
void DeviceClient::HandleDeviceControl(const DeviceControlRequest &) {}
void DeviceClient::HandleDeviceCommand(const DeviceCommand &) {}
void DeviceClient::HandleDeviceStatusUpdate(const std::string &, const DeviceStatusInfo &) {}
void DeviceClient::HandleDeviceData(const DeviceData &) {}

void DeviceClient::NotifyDeviceStatusChanged(const std::string &device_id, DeviceStatus old_status, DeviceStatus new_status) {
  if (event_handler_) event_handler_->OnDeviceStatusChanged(device_id, old_status, new_status);
}
void DeviceClient::NotifyDeviceDataReceived(const DeviceData &device_data) {
  if (event_handler_) event_handler_->OnDeviceDataReceived(device_data);
}
void DeviceClient::NotifyDeviceError(const std::string &device_id, uint16_t error_code, const std::string &error_message) {
  if (event_handler_) event_handler_->OnDeviceError(device_id, error_code, error_message);
}

DeviceClient::DeviceHandlerPtr DeviceClient::GetDeviceHandler(const std::string &device_id) {
  std::lock_guard<std::mutex> lk(handlers_mutex_);
  auto it = device_handlers_.find(device_id);
  if (it != device_handlers_.end()) return it->second;
  return nullptr;
}

bool DeviceClient::ValidateDeviceRegistration(const std::string &device_id) { return !device_id.empty(); }

void DeviceClient::AutoReconnect() {
  // 简化：省略
}

void DeviceClient::SendHeartbeat() {
  // 简化：省略
}

// 新增的通信处理方法
void DeviceClient::HandleServerMessage(const std::shared_ptr<perception::Message>& message) {
  // 处理来自服务器的消息
  if (!message) return;
  
  try {
    // 根据消息类型处理不同的服务器命令
    std::cout << "收到服务器消息: " << message->GetMessageId() << std::endl;
    
    // 这里需要根据具体的消息协议来实现设备命令处理
    // 例如：设备控制、状态查询等
  } catch (const std::exception& e) {
    std::cerr << "处理服务器消息异常: " << e.what() << std::endl;
  }
}

void DeviceClient::HandleServerConnection(const std::string& service_id, bool connected) {
  std::cout << "服务器连接状态变化: " << service_id << " " << (connected ? "已连接" : "已断开") << std::endl;
  
  if (connected) {
    connected_ = true;
    connected_server_id_ = service_id;
  } else {
    connected_ = false;
    connected_server_id_.clear();
  }
}

void DeviceClient::HandleCommunicationError(const std::string& service_id, uint16_t error_code) {
  std::cerr << "通信错误: " << service_id << " 错误码: " << error_code << std::endl;
  ++total_errors_;
}
