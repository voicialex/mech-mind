#pragma once

#include "EndpointService.hpp"
#include <string>
#include <sstream>

namespace perception {

class ServiceInspector {
public:
	static std::string DumpText(const EndpointService& svc) {
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
		return oss.str();
	}
};

} // namespace perception


