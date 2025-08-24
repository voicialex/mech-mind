#include "UdpServiceDiscovery.hpp"
#include "Logger.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using namespace perception;

UdpServiceDiscovery::UdpServiceDiscovery(const UdpServiceDiscoveryConfig &config)
    : config_(config),
      recv_io_(std::make_shared<asio::io_context>()),
      bcast_io_(std::make_shared<asio::io_context>()),
      start_time_(0) {
  LOG_INFO_STREAM << "UdpServiceDiscovery创建 - 本地地址: " << config_.local_address << ":" << config_.discovery_port
                  << ", 广播: " << (config_.enable_broadcast ? "启用" : "禁用");
}

UdpServiceDiscovery::~UdpServiceDiscovery() { Stop(); }

void UdpServiceDiscovery::Stop() {
  LOG_INFO_STREAM << "正在停止UDP服务发现";
  // 停接收侧
  if (receiving_) {
    receiving_ = false;
    if (recv_io_) recv_io_->stop();
    if (recv_socket_) {
      asio::error_code ec;
      recv_socket_->close(ec);
    }
    if (recv_thread_.joinable()) recv_thread_.join();
  }
  // 停广播侧
  if (broadcasting_) {
    broadcasting_ = false;
    if (bcast_timer_) bcast_timer_->cancel();
    if (bcast_io_) bcast_io_->stop();
    if (bcast_socket_) {
      asio::error_code ec;
      bcast_socket_->close(ec);
    }
    if (bcast_thread_.joinable()) bcast_thread_.join();
  }

  std::lock_guard<std::mutex> lock(services_mutex_);
  discovered_services_.clear();

  LOG_INFO_STREAM << "UDP服务发现已停止";
}

bool UdpServiceDiscovery::RegisterService(const EndpointIdentity &service_info) {
  local_service_info_ = service_info;
  local_service_info_.last_activity = GetCurrentTimestamp();
  EnsureBroadcastStarted();
  return true;
}

void UdpServiceDiscovery::UnregisterService() {
  if (!broadcasting_) return;

  local_service_info_ = EndpointIdentity{};
}

std::vector<EndpointIdentity> UdpServiceDiscovery::DiscoverServices(const std::string &service_name,
                                                                    ServiceDiscoveredCallback callback) {
  if (callback) {
    service_discovered_callback_ = std::move(callback);
  }
  EnsureReceiveStarted();
  std::lock_guard<std::mutex> lock(services_mutex_);
  CleanupExpiredServices();
  std::vector<EndpointIdentity> result;
  for (const auto &[id, service] : discovered_services_) {
    if (service_name.empty() || service.name == service_name) {
      result.push_back(service);
    }
  }
  return result;
}

// 回调通过 DiscoverServices 传入，无需独立注册接口

bool UdpServiceDiscovery::IsRunning() const { return receiving_ || broadcasting_; }

void UdpServiceDiscovery::StartReceive() {
  if (!recv_socket_) return;
  recv_socket_->async_receive_from(asio::buffer(recv_buffer_), recv_remote_endpoint_,
                                   [this](const asio::error_code &ec, std::size_t bytes_transferred) {
                                     if (!ec && bytes_transferred > 0) {
                                       std::vector<uint8_t> data(recv_buffer_.begin(),
                                                                 recv_buffer_.begin() + bytes_transferred);
                                       ProcessDiscoveryMessage(data, recv_remote_endpoint_);
                                     }
                                     if (receiving_) StartReceive();
                                   });
}

void UdpServiceDiscovery::StartBroadcastTimer() {
  if (!bcast_timer_) return;
  bcast_timer_->expires_after(std::chrono::milliseconds(config_.broadcast_interval));
  bcast_timer_->async_wait([this](const asio::error_code &ec) {
    if (!ec && broadcasting_) {
      BroadcastServiceInfo();
      StartBroadcastTimer();
    }
  });
}

void UdpServiceDiscovery::HandleSend(const asio::error_code &ec, std::size_t bytes_transferred) {
  if (!ec) {
    broadcasts_sent_++;
  }
}

void UdpServiceDiscovery::BroadcastServiceInfo() {
  if (local_service_info_.id.empty()) {
    return;
  }

  try {
    if (!bcast_socket_) return;
    auto data = SerializeServiceInfo(local_service_info_);
    asio::ip::udp::endpoint broadcast_endpoint(asio::ip::address_v4::broadcast(), config_.discovery_port);
    bcast_socket_->async_send_to(asio::buffer(data), broadcast_endpoint,
                                 [this](const asio::error_code &ec, std::size_t) { HandleSend(ec, 0); });

    LOG_DEBUG_STREAM << "[SD] 广播服务信息 -> id=" << local_service_info_.id
                     << ", address=" << local_service_info_.address << ":" << local_service_info_.port;
  } catch (const std::exception &e) {
    LOG_ERROR_STREAM << "[SD] 广播服务信息失败: " << e.what();
  }
}

void UdpServiceDiscovery::ProcessDiscoveryMessage(const std::vector<uint8_t> &data,
                                                  const asio::ip::udp::endpoint &sender) {
  try {
    std::string message(data.begin(), data.end());
    nlohmann::json json_msg = nlohmann::json::parse(message);

    std::string msg_type = json_msg["type"];

    if (msg_type == "service_discovery") {
      EndpointIdentity service_info = DeserializeServiceInfo(data);
      // 仅当地址为空或为0.0.0.0时回填发送方地址，避免被虚拟网卡地址覆盖
      if (service_info.address.empty() || service_info.address == "0.0.0.0") {
        service_info.address = sender.address().to_string();
      }
      service_info.last_activity = GetCurrentTimestamp();

      LOG_DEBUG_STREAM << "[SD] 收到服务发现消息 -> id=" << service_info.id << ", address=" << service_info.address
                       << ":" << service_info.port;

      ProcessServiceInfo(service_info);
    }
  } catch (const std::exception &e) {
    LOG_ERROR_STREAM << "[SD] 处理发现消息失败: " << e.what();
  }
}

void UdpServiceDiscovery::ProcessServiceInfo(const EndpointIdentity &service_info) {
  if (service_info.id == local_service_info_.id) {
    return;
  }

  bool is_new_service = false;
  {
    std::lock_guard<std::mutex> lock(services_mutex_);

    auto it = discovered_services_.find(service_info.id);
    if (it == discovered_services_.end()) {
      discovered_services_[service_info.id] = service_info;
      is_new_service = true;
      services_discovered_++;
    } else {
      it->second = service_info;
    }
  }

  if (is_new_service && service_discovered_callback_) {
    service_discovered_callback_(service_info);
  }
}

void UdpServiceDiscovery::CleanupExpiredServices() {
  uint64_t current_time = GetCurrentTimestamp();
  std::vector<std::string> expired_services;

  for (const auto &[id, service] : discovered_services_) {
    if (current_time - service.last_activity > config_.service_timeout) {
      expired_services.push_back(id);
    }
  }

  for (const auto &id : expired_services) {
    discovered_services_.erase(id);
  }
}

std::vector<uint8_t> UdpServiceDiscovery::SerializeServiceInfo(const EndpointIdentity &service_info) {
  nlohmann::json j;
  j["type"] = "service_discovery";
  j["id"] = service_info.id;
  j["name"] = service_info.name;
  j["address"] = service_info.address;
  j["port"] = service_info.port;
  j["timestamp"] = service_info.last_activity;
  j["service_type"] = static_cast<int>(service_info.type);

  std::string json_str = j.dump();
  // LOG_INFO_STREAM << "SerializeServiceInfo JSON: " << json_str; // Debug print
  return std::vector<uint8_t>(json_str.begin(), json_str.end());
}

EndpointIdentity UdpServiceDiscovery::DeserializeServiceInfo(const std::vector<uint8_t> &data) {
  try {
    std::string json_str(data.begin(), data.end());
    // LOG_INFO_STREAM << "DeserializeServiceInfo raw JSON: " << json_str;
    nlohmann::json j = nlohmann::json::parse(json_str);

    EndpointIdentity service_info;
    service_info.id = j.value("id", std::string{});
    service_info.name = j.value("name", std::string{});
    service_info.address = j.value("address", std::string{});
    service_info.port = j.value("port", 0);
    service_info.type = static_cast<EndpointType>(j.value("service_type", 0));
    service_info.last_activity = j.value("timestamp", GetCurrentTimestamp());

    return service_info;
  } catch (const std::exception &e) {
    return EndpointIdentity{};
  }
}

uint64_t UdpServiceDiscovery::GetCurrentTimestamp() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
      .count();
}

void UdpServiceDiscovery::EnsureReceiveStarted() {
  if (receiving_) return;
  try {
    if (!recv_socket_) {
      recv_socket_ = std::make_unique<asio::ip::udp::socket>(*recv_io_);
      recv_socket_->open(asio::ip::udp::v4());
      recv_socket_->set_option(asio::socket_base::reuse_address(true));
      recv_socket_->bind(
          asio::ip::udp::endpoint(asio::ip::make_address(config_.local_address), config_.discovery_port));
    }
    receiving_ = true;
    StartReceive();
    if (!recv_thread_.joinable()) {
      recv_thread_ = std::thread([ctx = recv_io_]() {
        try {
          ctx->run();
        } catch (const std::exception &e) {
          std::cerr << "UDP接收线程异常: " << e.what() << std::endl;
        }
      });
    }
    LOG_INFO_STREAM << "UDP服务发现接收已启动 - 端口: " << config_.discovery_port;
  } catch (const std::exception &e) {
    std::cerr << "启动接收失败: " << e.what() << std::endl;
  }
}

void UdpServiceDiscovery::EnsureBroadcastStarted() {
  if (!config_.enable_broadcast) return;
  if (broadcasting_) return;
  try {
    if (!bcast_socket_) {
      bcast_socket_ = std::make_unique<asio::ip::udp::socket>(*bcast_io_);
      bcast_socket_->open(asio::ip::udp::v4());
      bcast_socket_->set_option(asio::socket_base::broadcast(true));
    }
    if (!bcast_timer_) bcast_timer_ = std::make_unique<asio::steady_timer>(*bcast_io_);
    broadcasting_ = true;
    BroadcastServiceInfo();
    StartBroadcastTimer();
    if (!bcast_thread_.joinable()) {
      bcast_thread_ = std::thread([ctx = bcast_io_]() {
        try {
          ctx->run();
        } catch (const std::exception &e) {
          std::cerr << "UDP广播线程异常: " << e.what() << std::endl;
        }
      });
    }
    LOG_INFO_STREAM << "UDP服务广播已启动 - 端口: " << config_.discovery_port;
  } catch (const std::exception &e) {
    std::cerr << "启动广播失败: " << e.what() << std::endl;
  }
}
