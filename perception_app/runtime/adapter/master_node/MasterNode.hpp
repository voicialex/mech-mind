#pragma once

#include "communication/endpoints/servers/EndpointServer.hpp"
#include "communication/discovery/UdpServiceDiscovery.hpp"
#include "communication/interfaces/ConnectionTypes.hpp"
#include "Logger.hpp"
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>

namespace perception {

/**
 * @brief Master节点 - 包含服务发现和服务器端点
 * 
 * 设计原则：
 * - 作为中央服务器节点
 * - 管理服务发现广播
 * - 处理客户端连接
 * - 提供统一的服务器接口
 * - 维护所有客户端状态列表
 * - 使用统一的ConnectionTypes结构体，避免重复定义
 * - 主动监控客户端连接状态，维护上线离线关系
 * - 实现心跳机制和连接状态同步
 */
class MasterNode {
public:
	using Ptr = std::shared_ptr<MasterNode>;
	
	/**
	 * @brief Master节点配置 - 基于统一的EndpointConfig
	 */
	struct Config {
		// 服务发现配置
		std::string discovery_address = "0.0.0.0";
		uint16_t discovery_port = 8080;
		uint32_t broadcast_interval = 5000; // 毫秒
		
		// 设备服务器端点配置（使用统一的EndpointConfig）
		EndpointConfig device_server_config;
		
		// 客户端状态监控配置
		uint32_t client_timeout_interval = 60000;      // 客户端超时时间（毫秒）
		uint32_t status_check_interval = 10000;        // 状态检查间隔（毫秒）
		uint32_t state_sync_interval = 5000;           // 状态同步间隔（毫秒）
		bool enable_auto_cleanup = true;               // 是否启用自动清理离线客户端
		bool enable_heartbeat = true;                  // 是否启用心跳机制
		
		Config() {
			// 初始化设备服务器默认配置
			device_server_config.id = "device_server";
			device_server_config.name = "Device Server";
			device_server_config.address = "127.0.0.1";
			device_server_config.port = 9090;
			device_server_config.type = EndpointType::Server;
			device_server_config.max_clients = 100;
			device_server_config.client_timeout = 30000;
		}
	};

public:
	explicit MasterNode(const Config& config);
	~MasterNode();

	/**
	 * @brief 初始化Master节点
	 * @return 是否初始化成功
	 */
	bool Initialize();
	
	/**
	 * @brief 启动Master节点
	 * @return 是否启动成功
	 */
	bool Start();
	
	/**
	 * @brief 停止Master节点
	 */
	void Stop();
	
	
	/**
	 * @brief 获取所有客户端状态列表
	 * @return 客户端状态列表的副本
	 */
	std::vector<ConnectionInfo> GetClientList() const;
	
	/**
	 * @brief 获取指定客户端状态
	 * @param endpoint_id 客户端端点ID
	 * @return 客户端状态信息，如果不存在则返回空对象
	 */
	ConnectionInfo GetClientInfo(const std::string& endpoint_id) const;
	
	/**
	 * @brief 获取当前连接的客户端数量
	 * @return 连接中的客户端数量
	 */
	size_t GetConnectedClientCount() const;
	
	/**
	 * @brief 获取离线客户端数量
	 * @return 离线客户端数量
	 */
	size_t GetOfflineClientCount() const;
	
	/**
	 * @brief 断开指定客户端连接
	 * @param endpoint_id 客户端端点ID
	 * @return 是否成功断开
	 */
	bool DisconnectClient(const std::string& endpoint_id);
	
	/**
	 * @brief 强制清理离线客户端
	 * @return 清理的客户端数量
	 */
	size_t CleanupOfflineClients();
	
	/**
	 * @brief 检查客户端是否在线
	 * @param endpoint_id 客户端端点ID
	 * @return 是否在线
	 */
	bool IsClientOnline(const std::string& endpoint_id) const;
	
	/**
	 * @brief 获取客户端连接时长
	 * @param endpoint_id 客户端端点ID
	 * @return 连接时长（秒），如果客户端不存在则返回0
	 */
	uint64_t GetClientConnectionDuration(const std::string& endpoint_id) const;
	
	/**
	 * @brief 获取客户端最后活动时间
	 * @param endpoint_id 客户端端点ID
	 * @return 最后活动时间（毫秒时间戳），如果客户端不存在则返回0
	 */
	uint64_t GetClientLastActivity(const std::string& endpoint_id) const;
	

	
	/**
	 * @brief 强制同步客户端状态
	 * @return 同步的客户端数量
	 */
	size_t SyncClientStates();

	/**
	 * @brief 获取服务器实例
	 * @return 服务器指针
	 */
	std::shared_ptr<EndpointServer> GetServer() const { return server_; }

private:
	// 内部事件处理器适配器（直接处理事件并维护客户端状态）
	class MasterNodeEventHandler : public ITransport::EventHandler {
	public:
		explicit MasterNodeEventHandler(MasterNode* node) : node_(node) {}
		
		void OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) override {
			LOG_DEBUG_STREAM << "[RX] Master收到消息 <- 端点: " << endpoint_id
							<< ", size=" << message_data.size();
			// 统一由EndpointService层识别心跳并维护统计，这里只维护活动时间
			node_->UpdateClientActivity(endpoint_id);
		}
		
		void OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) override {
			if (connected) {
				LOG_DEBUG_STREAM << "[Server] 客户端连接成功 - 端点: " << endpoint_id
								<< ", 地址: " << connection_info.remote_endpoint.address
								<< ":" << connection_info.remote_endpoint.port;
				
				// 添加或更新客户端状态
				node_->AddOrUpdateClient(endpoint_id, connection_info);
			} else {
				LOG_DEBUG_STREAM << "[Server] 客户端断开连接 - 端点: " << endpoint_id
								<< ", 地址: " << connection_info.remote_endpoint.address
								<< ":" << connection_info.remote_endpoint.port;
				
				// 更新客户端断开状态
				node_->UpdateClientDisconnection(endpoint_id);
			}
		}
		
		void OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) override {
			LOG_ERROR_STREAM << "[Server] 错误 - 端点: " << endpoint_id 
						 	 << ", 错误码: " << error_code 
						 	 << ", 错误信息: " << error_message;
			
			// 更新客户端错误信息
			node_->UpdateClientError(endpoint_id, error_code, error_message);
		}
		
	private:
		MasterNode* node_;
	};

private:
	// 服务发现回调（保留）
	void OnServiceDiscovered(const EndpointIdentity& service_info);
	
	// 客户端状态管理方法
	void AddOrUpdateClient(const std::string& endpoint_id, const ConnectionInfo& connection_info);
	void UpdateClientDisconnection(const std::string& endpoint_id);
	void UpdateClientActivity(const std::string& endpoint_id);
	void UpdateClientError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message);
	
	// 客户端状态监控方法
	void StartStatusMonitoring();
	void StopStatusMonitoring();
	void StatusMonitoringLoop();
	void CheckClientTimeouts();
	void SyncClientStatesFromServer();
	

	
	// 工具方法
	uint64_t GetCurrentTimestamp() const;
	bool IsClientTimedOut(const ConnectionInfo& client_info) const;
	void UpdateClientStateFromServer(const EndpointIdentity& server_client);

private:
	Config config_;
	
	// 服务发现组件
	std::shared_ptr<UdpServiceDiscovery> service_discovery_;
	
	// 服务器端点组件
	std::shared_ptr<EndpointServer> server_;
	
	// 事件处理器
	std::shared_ptr<MasterNodeEventHandler> event_handler_;
	
	// 客户端状态管理（使用统一的ConnectionInfo）
	mutable std::mutex clients_mutex_;
	std::unordered_map<std::string, ConnectionInfo> clients_;
	
	// 状态监控线程
	std::atomic<bool> monitoring_running_{false};
	std::thread status_monitoring_thread_;
	
	// 心跳相关
	std::atomic<uint64_t> last_heartbeat_time_{0};
	
	// 状态管理
	bool initialized_ = false;
	bool running_ = false;
};

} // namespace perception
