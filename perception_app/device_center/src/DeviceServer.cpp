#include "../include/DeviceServer.hpp"
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

DeviceServer::DeviceServer(const Config &config) : config_(config) {}
DeviceServer::DeviceServer() : config_() {}  // 默认构造函数使用默认配置
DeviceServer::~DeviceServer() { Stop(); }

// 实现通信删除器
void DeviceServer::CommunicationDeleter::operator()(void *ptr) const {
  if (ptr) {
    // 在实际应用中，这里应该进行正确的类型转换
    // 例如：delete static_cast<perception::CommunicationInterface*>(ptr);
    delete ptr;
  }
}

bool DeviceServer::Initialize(const std::string &config_path) {
  if (initialized_) return true;
  LoadConfig(config_path);
  InitializeCommunication();
  server_start_time_ = now_ms();
  initialized_ = true;
  return true;
}

bool DeviceServer::Start() {
  if (!initialized_) return false;
  if (running_) return true;
  running_ = true;
  StartHeartbeatMonitor();
  return true;
}

void DeviceServer::Stop() {
  if (!running_) return;
  running_ = false;
  StopHeartbeatMonitor();
}

void DeviceServer::Run() {
  // 简化：不启动额外循环，交由应用侧控制
  std::cout << "设备服务器运行中，等待客户端连接..." << std::endl;
}

void DeviceServer::Cleanup() {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  devices_.clear();
  device_status_.clear();
  device_clients_.clear();
  client_devices_.clear();
}

bool DeviceServer::RegisterDeviceHandler(DeviceType device_type, DeviceHandlerPtr handler) {
  std::lock_guard<std::mutex> lk(handlers_mutex_);
  device_handlers_[device_type] = std::move(handler);
  return true;
}

bool DeviceServer::RegisterDeviceHandler(const std::string &device_type_name, DeviceHandlerPtr handler) {
  std::lock_guard<std::mutex> lk(handlers_mutex_);
  device_handlers_by_name_[device_type_name] = std::move(handler);
  return true;
}

void DeviceServer::UnregisterDeviceHandler(DeviceType device_type) {
  std::lock_guard<std::mutex> lk(handlers_mutex_);
  device_handlers_.erase(device_type);
}

void DeviceServer::RegisterEventHandler(EventHandlerPtr handler) {
  std::lock_guard<std::mutex> lk(handlers_mutex_);
  event_handler_ = std::move(handler);
}

DeviceRegisterResponse DeviceServer::RegisterDevice(const DeviceRegisterRequest &request) {
  DeviceRegisterResponse resp{};
  if (!ValidateDeviceRegistration(request)) {
    resp.success = false;
    resp.message = "invalid registration";
    resp.error_code = 1;
    return resp;
  }

  const std::string device_id = request.device_id.empty() ? GenerateDeviceId(request) : request.device_id;

  DeviceInfo info{};
  info.device_id = device_id;
  info.device_name = request.device_name;
  info.device_type = request.device_type;
  info.device_model = request.device_model;
  info.device_version = request.device_version;
  info.capabilities = request.capabilities;
  info.status = DeviceStatus::Online;
  info.status_message = "registered";
  info.last_heartbeat = now_ms();

  {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    devices_[device_id] = info;
    device_clients_[device_id] = request.client_id;
    client_devices_[request.client_id].push_back(device_id);
    ++total_devices_registered_;
    ++total_devices_online_;
  }

  resp.success = true;
  resp.message = "ok";
  resp.device_id = device_id;
  resp.error_code = 0;

  // 回传部分服务配置
  nlohmann::json server_config_json;
  server_config_json["service_id"] = config_.service_id;
  server_config_json["service_name"] = config_.service_name;
  server_config_json["port"] = config_.local_port;
  resp.server_config = std::move(server_config_json);
  return resp;
}

bool DeviceServer::UnregisterDevice(const std::string &device_id) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  auto it = devices_.find(device_id);
  if (it == devices_.end()) return false;
  auto client_it = device_clients_.find(device_id);
  if (client_it != device_clients_.end()) {
    auto &vec = client_devices_[client_it->second];
    vec.erase(std::remove(vec.begin(), vec.end(), device_id), vec.end());
    device_clients_.erase(client_it);
  }
  devices_.erase(it);
  device_status_.erase(device_id);
  return true;
}

DeviceDiscoveryResponse DeviceServer::DiscoverDevices(const DeviceDiscoveryRequest &request) {
  DeviceDiscoveryResponse resp{};
  resp.success = true;
  resp.error_code = 0;
  std::lock_guard<std::mutex> lk(devices_mutex_);
  for (const auto &kv : devices_) {
    const auto &info = kv.second;
    if (!request.include_offline && info.status != DeviceStatus::Online) continue;
    if (request.device_type != DeviceType::Unknown && info.device_type != request.device_type) continue;
    resp.devices.push_back(info);
  }
  resp.message = "ok";
  return resp;
}

DeviceInfo DeviceServer::GetDeviceInfo(const std::string &device_id) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  auto it = devices_.find(device_id);
  if (it != devices_.end()) return it->second;
  return DeviceInfo{};
}

DeviceStatusInfo DeviceServer::GetDeviceStatus(const std::string &device_id) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  auto it = device_status_.find(device_id);
  if (it != device_status_.end()) return it->second;
  DeviceStatusInfo s{};
  s.device_id = device_id;
  s.status = DeviceStatus::Offline;
  s.timestamp = now_ms();
  return s;
}

DeviceControlResponse DeviceServer::ControlDevice(const DeviceControlRequest &request) {
  DeviceControlResponse resp{};
  auto handler = GetDeviceHandler(request.device_id);
  if (!handler) {
    resp.success = false;
    resp.message = "no handler";
    resp.error_code = 2;
    resp.device_id = request.device_id;
    return resp;
  }
  DeviceCommand cmd{};
  cmd.device_id = request.device_id;
  cmd.command_id = std::to_string(now_ms());
  cmd.command_type = request.command_type;
  cmd.parameters = request.parameters;
  cmd.timestamp = now_ms();
  cmd.timeout_ms = request.timeout_ms;
  cmd.require_response = request.require_response;

  auto r = handler->HandleCommand(cmd);
  resp.success = r.success;
  resp.message = r.message;
  resp.device_id = request.device_id;
  resp.command_type = request.command_type;
  resp.result = r.data;
  resp.error_code = r.error_code;
  ++total_commands_processed_;
  return resp;
}

DeviceResponse DeviceServer::SendDeviceCommand(const DeviceCommand &command) {
  auto handler = GetDeviceHandler(command.device_id);
  if (!handler) {
    DeviceResponse r{};
    r.device_id = command.device_id;
    r.command_id = command.command_id;
    r.success = false;
    r.message = "no handler";
    r.timestamp = now_ms();
    r.error_code = 2;
    return r;
  }
  ++total_commands_processed_;
  return handler->HandleCommand(command);
}

bool DeviceServer::UpdateDeviceStatus(const std::string &device_id, const DeviceStatusInfo &status) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  device_status_[device_id] = status;
  auto it = devices_.find(device_id);
  if (it != devices_.end()) {
    auto old = it->second.status;
    it->second.status = status.status;
    it->second.status_message = status.status_message;
    it->second.last_heartbeat = status.timestamp;
    if (event_handler_ && old != status.status) {
      event_handler_->OnDeviceStatusChanged(device_id, old, status.status);
    }
  }
  return true;
}

void DeviceServer::ReceiveDeviceData(const DeviceData &device_data) {
  ++total_data_received_;
  if (event_handler_) {
    event_handler_->OnDeviceDataReceived(device_data);
  }
}

void DeviceServer::BroadcastDeviceData(const DeviceData &device_data, const std::vector<std::string> &) {
  // 简化：仅触发事件
  if (event_handler_) {
    event_handler_->OnDeviceDataReceived(device_data);
  }
}

std::vector<DeviceInfo> DeviceServer::GetAllDevices() const {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  std::vector<DeviceInfo> v;
  v.reserve(devices_.size());
  for (const auto &kv : devices_) v.push_back(kv.second);
  return v;
}

std::vector<DeviceInfo> DeviceServer::GetOnlineDevices() const {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  std::vector<DeviceInfo> v;
  for (const auto &kv : devices_)
    if (kv.second.status == DeviceStatus::Online) v.push_back(kv.second);
  return v;
}

std::map<DeviceType, uint32_t> DeviceServer::GetDeviceTypeStatistics() const {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  std::map<DeviceType, uint32_t> stats;
  for (const auto &kv : devices_) stats[kv.second.device_type]++;
  return stats;
}

bool DeviceServer::IsDeviceOnline(const std::string &device_id) const {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  auto it = devices_.find(device_id);
  return it != devices_.end() && it->second.status == DeviceStatus::Online;
}

bool DeviceServer::HasDevice(const std::string &device_id) const {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  return devices_.count(device_id) > 0;
}

nlohmann::json DeviceServer::GetServerStatistics() const {
  std::lock_guard<std::mutex> lk(stats_mutex_);
  nlohmann::json stats_json;
  stats_json["total_devices_registered"] = total_devices_registered_;
  stats_json["total_devices_online"] = total_devices_online_;
  stats_json["total_commands_processed"] = total_commands_processed_;
  stats_json["total_data_received"] = total_data_received_;
  stats_json["total_errors"] = total_errors_;
  stats_json["uptime_ms"] = (server_start_time_ == 0) ? 0 : (now_ms() - server_start_time_);
  return stats_json;
}

DeviceServer::Config DeviceServer::GetConfig() const { return config_; }

bool DeviceServer::IsRunning() const { return running_; }

bool DeviceServer::IsInitialized() const { return initialized_; }

void DeviceServer::LoadConfig(const std::string &config_path) {
  if (config_path.empty()) return;
  try {
    std::ifstream ifs(config_path);
    if (!ifs.is_open()) return;
    nlohmann::json config_json;
    ifs >> config_json;
    if (config_json.contains("server")) {
      const auto &server_config = config_json["server"];
      if (server_config.contains("service_id")) config_.service_id = server_config["service_id"].get<std::string>();
      if (server_config.contains("service_name")) config_.service_name = server_config["service_name"].get<std::string>();
      if (server_config.contains("local_address")) config_.local_address = server_config["local_address"].get<std::string>();
      if (server_config.contains("local_port")) config_.local_port = server_config["local_port"].get<uint16_t>();
    }
  } catch (...) {
  }
}

void DeviceServer::InitializeCommunication() {
  // 简化：此处保持空实现，作为与通信层的解耦占位
}

void DeviceServer::StartHeartbeatMonitor() {
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

void DeviceServer::StopHeartbeatMonitor() {
  if (heartbeat_thread_.joinable()) heartbeat_thread_.join();
}

void DeviceServer::ProcessHeartbeat() {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  const uint64_t t = now_ms();
  for (auto &kv : devices_) {
    auto &info = kv.second;
    if (t - info.last_heartbeat > config_.device_timeout) {
      auto old = info.status;
      info.status = DeviceStatus::Offline;
      if (event_handler_ && old != info.status) {
        event_handler_->OnDeviceStatusChanged(info.device_id, old, info.status);
      }
    }
  }
}

void DeviceServer::CleanupOfflineDevices() {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  for (auto it = devices_.begin(); it != devices_.end();) {
    if (it->second.status == DeviceStatus::Offline)
      it = devices_.erase(it);
    else
      ++it;
  }
}

void DeviceServer::HandleDeviceRegistration(const DeviceRegisterRequest &) {}
void DeviceServer::HandleDeviceDiscovery(const DeviceDiscoveryRequest &) {}
void DeviceServer::HandleDeviceControl(const DeviceControlRequest &) {}
void DeviceServer::HandleDeviceCommand(const DeviceCommand &) {}
void DeviceServer::HandleDeviceStatusUpdate(const std::string &, const DeviceStatusInfo &) {}
void DeviceServer::HandleDeviceData(const DeviceData &) {}

void DeviceServer::NotifyDeviceStatusChanged(const std::string &device_id, DeviceStatus old_status, DeviceStatus new_status) {
  if (event_handler_) event_handler_->OnDeviceStatusChanged(device_id, old_status, new_status);
}
void DeviceServer::NotifyDeviceDataReceived(const DeviceData &device_data) {
  if (event_handler_) event_handler_->OnDeviceDataReceived(device_data);
}
void DeviceServer::NotifyDeviceError(const std::string &device_id, uint16_t error_code, const std::string &error_message) {
  if (event_handler_) event_handler_->OnDeviceError(device_id, error_code, error_message);
}

DeviceServer::DeviceHandlerPtr DeviceServer::GetDeviceHandler(DeviceType device_type) {
  std::lock_guard<std::mutex> lk(handlers_mutex_);
  auto it = device_handlers_.find(device_type);
  if (it != device_handlers_.end()) return it->second;
  return nullptr;
}

DeviceServer::DeviceHandlerPtr DeviceServer::GetDeviceHandler(const std::string &device_id) {
  std::lock_guard<std::mutex> lk(devices_mutex_);
  auto it = devices_.find(device_id);
  if (it == devices_.end()) return nullptr;
  auto type = it->second.device_type;
  return GetDeviceHandler(type);
}

bool DeviceServer::ValidateDeviceRegistration(const DeviceRegisterRequest &request) { return !request.device_name.empty(); }

std::string DeviceServer::GenerateDeviceId(const DeviceRegisterRequest &request) {
  std::ostringstream oss;
  oss << static_cast<int>(request.device_type) << "-" << request.device_name << "-" << now_ms();
  return oss.str();
}
