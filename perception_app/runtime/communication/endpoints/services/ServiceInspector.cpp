#include "ServiceInspector.hpp"
#include "communication/endpoints/servers/EndpointServer.hpp"
#include <chrono>
#include <algorithm>
#include <iomanip>

namespace perception {

std::string ServiceInspector::DumpText(const EndpointService &svc) {
  std::ostringstream oss;
  oss << "EndpointService Statistics:\n";
  oss << "  State: " << static_cast<int>(svc.state_.load()) << "\n";
  oss << "  Initialized: " << (svc.initialized_.load() ? "true" : "false") << "\n";
  oss << "  Running: " << (svc.running_.load() ? "true" : "false") << "\n";
  oss << "  Messages Sent: " << svc.statistics_.messages_sent.load() << "\n";
  oss << "  Messages Received: " << svc.statistics_.messages_received.load() << "\n";
  oss << "  Connections Established: " << svc.statistics_.connections_established.load() << "\n";
  oss << "  Errors: " << svc.statistics_.errors.load() << "\n";
  oss << "  Running Time: " << svc.statistics_.GetUptime() << " ms\n";
  {
    std::lock_guard<std::mutex> lock(svc.connections_mutex_);
    oss << "  Active Connections: " << svc.endpoint_connections_.size() << "\n";
  }

  // 心跳统计信息
  if (svc.IsHeartbeatEnabled()) {
    oss << "  Heartbeat Enabled: true\n";

    // 显示客户端心跳信息（仅服务器）
    if (svc.config_.type == EndpointType::Server) {
      const auto *server = dynamic_cast<const EndpointServer *>(&svc);
      if (server) {
        auto heartbeat_info = server->GetAllClientHeartbeatInfo();
        oss << "  Client Heartbeat Info (" << heartbeat_info.size() << " clients):\n";
        for (const auto &[client_id, info] : heartbeat_info) {
          oss << "    Client " << client_id << ":\n";
          oss << "      Alive: " << (info.is_alive ? "true" : "false") << "\n";
          oss << "      Total Requests: " << info.total_requests << "\n";
          oss << "      Total Responses: " << info.total_responses << "\n";
          oss << "      Response Rate: " << std::fixed << std::setprecision(2) << (info.GetResponseRate() * 100)
              << "%\n";
          oss << "      Consecutive Missed: " << info.consecutive_missed << "\n";
          oss << "      Last Request Time: " << info.last_request_time << " ms\n";
          oss << "      Last Response Time: " << info.last_response_time << " ms\n";
          oss << "      Last Activity: " << info.GetLastActivity() << " ms\n";
        }
      }
    }
  } else {
    oss << "  Heartbeat Enabled: false\n";
  }

  return oss.str();
}

std::string ServiceInspector::GetHeartbeatStatistics(const EndpointService &svc) {
  if (!svc.IsHeartbeatEnabled()) {
    return "Heartbeat is disabled";
  }

  std::ostringstream oss;

  if (svc.config_.type == EndpointType::Server) {
    const auto *server = dynamic_cast<const EndpointServer *>(&svc);
    if (server) {
      auto heartbeat_info = server->GetAllClientHeartbeatInfo();

      oss << "=== Server Heartbeat Statistics ===\n";
      oss << "Service ID: " << svc.config_.id << "\n";
      oss << "Total Clients: " << heartbeat_info.size() << "\n";

      uint32_t alive_count = 0;
      uint32_t total_requests = 0;
      uint32_t total_responses = 0;

      // 按客户端ID排序，确保输出一致性
      std::vector<std::pair<std::string, EndpointService::ClientHeartbeatInfo>> sorted_info;
      for (const auto &[client_id, info] : heartbeat_info) {
        sorted_info.emplace_back(client_id, info);
      }
      std::sort(sorted_info.begin(), sorted_info.end());

      for (const auto &[client_id, info] : sorted_info) {
        if (info.is_alive) alive_count++;
        total_requests += info.total_requests;
        total_responses += info.total_responses;

        oss << "\nClient: " << client_id << "\n";
        oss << "  Status: " << (info.is_alive ? "Alive" : "Dead") << "\n";
        oss << "  Requests: " << info.total_requests << "\n";
        oss << "  Responses: " << info.total_responses << "\n";
        oss << "  Response Rate: " << std::fixed << std::setprecision(1) << (info.GetResponseRate() * 100) << "%\n";
        oss << "  Consecutive Missed: " << info.consecutive_missed << "\n";
      }

      oss << "\n=== Summary ===\n";
      oss << "Alive Clients: " << alive_count << "/" << heartbeat_info.size() << "\n";
      oss << "Total Requests: " << total_requests << "\n";
      oss << "Total Responses: " << total_responses << "\n";
      if (total_requests > 0) {
        oss << "Overall Response Rate: " << std::fixed << std::setprecision(1)
            << (static_cast<double>(total_responses) / total_requests * 100) << "%\n";
      }
    }
  } else {
    oss << "=== Client Heartbeat Statistics ===\n";
    oss << "Service ID: " << svc.config_.id << "\n";
    oss << "Heartbeat Enabled: " << (svc.IsHeartbeatEnabled() ? "Yes" : "No") << "\n";

    // 客户端主要关注连接状态
    {
      std::lock_guard<std::mutex> lock(svc.connections_mutex_);
      oss << "Connected Endpoints: " << svc.endpoint_connections_.size() << "\n";
    }

    // 添加基本统计信息
    oss << "\n=== Basic Statistics ===\n";
    oss << "Messages Sent: " << svc.statistics_.messages_sent.load() << "\n";
    oss << "Messages Received: " << svc.statistics_.messages_received.load() << "\n";
    oss << "Running Time: " << svc.statistics_.GetUptime() << " ms\n";
  }

  return oss.str();
}

bool ServiceInspector::IsClientHeartbeatAlive(const EndpointService &svc, const std::string &client_id) {
  if (svc.config_.type == EndpointType::Server) {
    const auto *server = dynamic_cast<const EndpointServer *>(&svc);
    if (server) {
      const auto *heartbeat_info = server->GetClientHeartbeatInfo(client_id);
      return heartbeat_info ? heartbeat_info->is_alive : false;
    }
  }
  return false;
}

const void *ServiceInspector::GetClientHeartbeatInfo(const EndpointService &svc, const std::string &client_id) {
  if (svc.config_.type == EndpointType::Server) {
    const auto *server = dynamic_cast<const EndpointServer *>(&svc);
    if (server) {
      return server->GetClientHeartbeatInfo(client_id);
    }
  }
  return nullptr;
}

}  // namespace perception
