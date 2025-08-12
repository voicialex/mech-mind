#include "../include/DeviceClient.hpp"
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
void DeviceClient::CommunicationDeleter::operator()(void *ptr) const {
  if (ptr) {
    // 在实际应用中，这里应该进行正确的类型转换
    // 例如：delete static_cast<perception::CommunicationInterface*>(ptr);
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
  connected_ = true;  // 简化：直接标记为已连接
  return true;
}

void DeviceClient::DisconnectFromServer() { connected_ = false; }

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

bool DeviceClient::RegisterDevice(const std::string &device_id, DeviceType device_type, const nlohmann::json &config) {
  // 先本地注册
  DeviceConfig dc;
  dc.device_id = device_id;
  dc.device_name = device_id;
  dc.device_type = device_type;
  dc.config = config;
  bool local_success = RegisterDevice(dc);

  // 如果已连接到服务器，向服务器注册设备
  if (connected_ && local_success) {
    DeviceRegisterRequest request;
    request.device_id = device_id;
    request.device_name = device_id;
    request.device_type = device_type;
    request.client_id = config_.service_id;
    request.capabilities = {DeviceCapability::Capture, DeviceCapability::Configure, DeviceCapability::Status};

    // 模拟向服务器发送注册请求
    std::cout << "向服务器注册设备: " << device_id << std::endl;

    // 这里应该通过通信接口发送请求到服务器
    // 简化实现：直接模拟成功
    return true;
  }

  return local_success;
}

bool DeviceClient::RegisterDevice(const DeviceConfig &device_config) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  DeviceInfo info{};
  info.device_id = device_config.device_id;
  info.device_name = device_config.device_name;
  info.device_type = device_config.device_type;
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
  // 简化：返回空或本地
  return {"local"};
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
    }
  } catch (...) {
  }
}

void DeviceClient::InitializeCommunication() {
  // 简化：与通信层解耦，占位
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

void DeviceClient::HandleServerMessage(const std::vector<uint8_t> &) {}
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

bool DeviceClient::ValidateDeviceRegistration(const std::string &device_id, DeviceType /*device_type*/) { return !device_id.empty(); }

void DeviceClient::AutoReconnect() {
  // 简化：省略
}

void DeviceClient::SendHeartbeat() {
  // 简化：省略
}
