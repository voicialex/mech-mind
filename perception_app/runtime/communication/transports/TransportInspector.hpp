#pragma once

#include "AsioTransport.hpp"
#include <nlohmann/json.hpp>

namespace perception {

class TransportInspector {
public:
	static std::string DumpJson(const AsioTransport& t) {
		nlohmann::json stats;
		stats["running"] = t.running_.load();
		stats["start_time"] = t.start_time_;
		stats["uptime_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count() - t.start_time_;
		stats["messages_sent"] = t.messages_sent_.load();
		stats["messages_received"] = t.messages_received_.load();
		stats["connection_errors"] = t.connection_errors_.load();
		{
			std::lock_guard<std::mutex> lock(t.connections_mutex_);
			stats["connections"] = t.connections_.size();
		}
		return stats.dump(2);
	}
};

} // namespace perception


