#include "DeviceManager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <nlohmann/json.hpp>

using namespace device_center;

namespace {
uint64_t now_ms() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
}  // namespace

DeviceManager::DeviceManager(const Config& config) : config_(config), start_time_(0) {}

DeviceManager::DeviceManager() : start_time_(0) {}

DeviceManager::~DeviceManager() { 
    Stop(); 
}

bool DeviceManager::Initialize(const std::string& config_path) {
    if (initialized_) return true;
    
    LoadConfig(config_path);
    InitializeCommunication();
    start_time_ = now_ms();
    initialized_ = true;
    return true;
}

bool DeviceManager::Start() {
    if (!initialized_) return false;
    if (running_) return true;
    
    running_ = true;
    StartHeartbeatMonitor();
    return true;
}

void DeviceManager::Stop() {
    if (!running_) return;
    
    running_ = false;
    StopHeartbeatMonitor();
}

void DeviceManager::Cleanup() {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    devices_.clear();
}

void DeviceManager::RegisterEventHandler(EventHandlerPtr handler) {
    std::lock_guard<std::mutex> lk(event_handler_mutex_);
    event_handler_ = std::move(handler);
}

bool DeviceManager::RegisterDevice(const std::string& device_id, 
                                 const DeviceInfo& device_info,
                                 const nlohmann::json& config) {
    if (!ValidateDeviceRegistration(device_id)) {
        return false;
    }

    DeviceContext context;
    context.info = device_info;
    context.info.device_id = device_id;
    context.info.status = DeviceStatus::Online;
    context.info.status_message = "registered";
    context.info.last_heartbeat = now_ms();
    
    context.status.device_id = device_id;
    context.status.status = DeviceStatus::Online;
    context.status.status_message = "registered";
    context.status.timestamp = now_ms();
    context.is_connected = true;
    context.last_heartbeat = now_ms();
    context.config = config;

    {
        std::lock_guard<std::mutex> lk(devices_mutex_);
        devices_[device_id] = context;
    }

    total_devices_registered_++;

    // 通知事件处理器
    NotifyDeviceConnectionChanged(device_id, true);
    NotifyDeviceStatusChanged(device_id, DeviceStatus::Offline, DeviceStatus::Online);

    return true;
}

bool DeviceManager::UnregisterDevice(const std::string& device_id) {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    auto it = devices_.find(device_id);
    if (it == devices_.end()) return false;

    // 通知事件处理器
    NotifyDeviceConnectionChanged(device_id, false);

    devices_.erase(it);
    return true;
}

bool DeviceManager::UpdateDeviceStatus(const std::string& device_id, const DeviceStatusInfo& status) {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    auto it = devices_.find(device_id);
    if (it == devices_.end()) return false;

    auto old_status = it->second.status.status;
    it->second.status = status;
    it->second.info.status = status.status;
    it->second.info.status_message = status.status_message;
    it->second.last_heartbeat = now_ms();

    // 通知事件处理器
    if (old_status != status.status) {
        NotifyDeviceStatusChanged(device_id, old_status, status.status);
    }

    return true;
}

DeviceInfo DeviceManager::GetDeviceInfo(const std::string& device_id) const {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    auto it = devices_.find(device_id);
    if (it != devices_.end()) {
        return it->second.info;
    }
    return DeviceInfo{}; // 返回空设备信息
}

DeviceStatusInfo DeviceManager::GetDeviceStatus(const std::string& device_id) const {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    auto it = devices_.find(device_id);
    if (it != devices_.end()) {
        return it->second.status;
    }
    return DeviceStatusInfo{}; // 返回空状态信息
}

std::vector<DeviceInfo> DeviceManager::GetAllDevices() const {
    std::vector<DeviceInfo> result;
    std::lock_guard<std::mutex> lk(devices_mutex_);
    for (const auto& kv : devices_) {
        result.push_back(kv.second.info);
    }
    return result;
}

std::vector<DeviceInfo> DeviceManager::GetOnlineDevices() const {
    std::vector<DeviceInfo> result;
    std::lock_guard<std::mutex> lk(devices_mutex_);
    for (const auto& kv : devices_) {
        if (kv.second.is_connected && kv.second.status.status == DeviceStatus::Online) {
            result.push_back(kv.second.info);
        }
    }
    return result;
}

bool DeviceManager::HasDevice(const std::string& device_id) const {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    return devices_.find(device_id) != devices_.end();
}

bool DeviceManager::IsDeviceOnline(const std::string& device_id) const {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    auto it = devices_.find(device_id);
    if (it != devices_.end()) {
        return it->second.is_connected && it->second.status.status == DeviceStatus::Online;
    }
    return false;
}

nlohmann::json DeviceManager::GetStatistics() const {
    nlohmann::json stats;
    stats["service_id"] = config_.service_id;
    stats["service_name"] = config_.service_name;
    stats["start_time"] = start_time_;
    stats["uptime_ms"] = now_ms() - start_time_;
    stats["total_devices_registered"] = total_devices_registered_.load();
    stats["total_messages"] = total_messages_.load();
    stats["total_errors"] = total_errors_.load();
    
    std::lock_guard<std::mutex> lk(devices_mutex_);
    stats["total_devices"] = devices_.size();
    
    int online_count = 0;
    for (const auto& kv : devices_) {
        if (kv.second.is_connected && kv.second.status.status == DeviceStatus::Online) {
            online_count++;
        }
    }
    stats["online_devices"] = online_count;
    
    return stats;
}

void DeviceManager::NotifyDeviceConnectionChanged(const std::string& device_id, bool connected) {
    std::lock_guard<std::mutex> lk(event_handler_mutex_);
    if (event_handler_) {
        event_handler_->OnDeviceConnectionChanged(device_id, connected);
    }
}

void DeviceManager::NotifyDeviceStatusChanged(const std::string& device_id, 
                                           DeviceStatus old_status, 
                                           DeviceStatus new_status) {
    std::lock_guard<std::mutex> lk(event_handler_mutex_);
    if (event_handler_) {
        event_handler_->OnDeviceStatusChanged(device_id, old_status, new_status);
    }
}

void DeviceManager::NotifyDeviceDataReceived(const DeviceData& device_data) {
    std::lock_guard<std::mutex> lk(event_handler_mutex_);
    if (event_handler_) {
        event_handler_->OnDeviceDataReceived(device_data);
    }
    total_messages_++;
}

void DeviceManager::NotifyDeviceError(const std::string& device_id, 
                                   uint16_t error_code, 
                                   const std::string& error_message) {
    std::lock_guard<std::mutex> lk(event_handler_mutex_);
    if (event_handler_) {
        event_handler_->OnDeviceError(device_id, error_code, error_message);
    }
    total_errors_++;
}

void DeviceManager::LoadConfig(const std::string& config_path) {
    if (config_path.empty()) return;
    
    try {
        std::ifstream file(config_path);
        if (file.is_open()) {
            nlohmann::json j;
            file >> j;
            
            if (j.contains("service_id")) config_.service_id = j["service_id"];
            if (j.contains("service_name")) config_.service_name = j["service_name"];
            if (j.contains("local_address")) config_.local_address = j["local_address"];
            if (j.contains("local_port")) config_.local_port = j["local_port"];
            if (j.contains("discovery_port")) config_.discovery_port = j["discovery_port"];
            if (j.contains("heartbeat_interval")) config_.heartbeat_interval = j["heartbeat_interval"];
            if (j.contains("device_timeout")) config_.device_timeout = j["device_timeout"];
            if (j.contains("enable_auto_cleanup")) config_.enable_auto_cleanup = j["enable_auto_cleanup"];
            if (j.contains("cleanup_interval")) config_.cleanup_interval = j["cleanup_interval"];
            if (j.contains("log_level")) config_.log_level = j["log_level"];
            if (j.contains("log_file")) config_.log_file = j["log_file"];
        }
    } catch (const std::exception &e) {
        std::cerr << "加载配置文件失败: " << e.what() << std::endl;
    }
}

void DeviceManager::InitializeCommunication() {
    // 创建通信适配器
    DeviceCommunicationAdapter::Config comm_config;
    comm_config.service_id = config_.service_id;
    comm_config.service_name = config_.service_name;
    comm_config.local_address = config_.local_address;
    comm_config.local_port = config_.local_port;
    comm_config.discovery_port = config_.discovery_port;
    comm_config.heartbeat_interval = config_.heartbeat_interval;
    comm_config.connection_timeout = config_.device_timeout;
    
    comm_adapter_ = std::make_shared<DeviceCommunicationAdapter>(comm_config);
    
    // 注册回调
    comm_adapter_->RegisterMessageCallback([this](const DeviceCommunicationAdapter::DeviceMessage& message) {
        OnMessageReceived(message);
    });
    
    comm_adapter_->RegisterConnectionCallback([this](const std::string& service_id, 
                                                   DeviceCommunicationAdapter::ConnectionState state) {
        OnConnectionStateChanged(service_id, state);
    });
    
    comm_adapter_->RegisterErrorCallback([this](const std::string& service_id, uint16_t error_code) {
        OnCommunicationError(service_id, error_code);
    });
    
    if (comm_adapter_->Initialize() && comm_adapter_->Start()) {
        std::cout << "通信适配器初始化成功" << std::endl;
    } else {
        std::cerr << "通信适配器初始化失败" << std::endl;
    }
}

void DeviceManager::StartHeartbeatMonitor() {
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

void DeviceManager::StopHeartbeatMonitor() {
    if (heartbeat_thread_.joinable()) heartbeat_thread_.join();
}

void DeviceManager::ProcessHeartbeat() {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    const uint64_t t = now_ms();
    for (auto &kv : devices_) {
        auto &context = kv.second;
        if (t - context.last_heartbeat > config_.device_timeout) {
            auto old_status = context.status.status;
            context.status.status = DeviceStatus::Offline;
            context.info.status = DeviceStatus::Offline;
            context.is_connected = false;
            
            // 通知事件处理器
            if (old_status != DeviceStatus::Offline) {
                NotifyDeviceStatusChanged(context.info.device_id, old_status, DeviceStatus::Offline);
                NotifyDeviceConnectionChanged(context.info.device_id, false);
            }
        }
    }
}

void DeviceManager::CleanupOfflineDevices() {
    std::lock_guard<std::mutex> lk(devices_mutex_);
    for (auto it = devices_.begin(); it != devices_.end();) {
        if (it->second.status.status == DeviceStatus::Offline)
            it = devices_.erase(it);
        else
            ++it;
    }
}

void DeviceManager::OnMessageReceived(const DeviceCommunicationAdapter::DeviceMessage& message) {
    HandleDeviceMessage(message);
}

void DeviceManager::OnConnectionStateChanged(const std::string& service_id, 
                                          DeviceCommunicationAdapter::ConnectionState state) {
    HandleConnectionStateChanged(service_id, state);
}

void DeviceManager::OnCommunicationError(const std::string& service_id, uint16_t error_code) {
    HandleCommunicationError(service_id, error_code);
}

bool DeviceManager::ValidateDeviceRegistration(const std::string& device_id) {
    return !device_id.empty();
}

std::string DeviceManager::GenerateDeviceId(const std::string& device_name) {
    return device_name + "_" + std::to_string(now_ms());
}

uint64_t DeviceManager::GetCurrentTimestamp() {
    return now_ms();
}
