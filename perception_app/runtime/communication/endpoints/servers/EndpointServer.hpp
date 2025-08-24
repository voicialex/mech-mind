#pragma once

#include "communication/endpoints/services/EndpointService.hpp"
#include "communication/endpoints/services/ServiceInspector.hpp"
#include "communication/interfaces/ConnectionTypes.hpp"
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <thread>

namespace perception {

/**
 * @brief 端点服务器 - 基于端点服务的服务器实现
 * 
 * 设计原则：
 * - 继承自EndpointService基类
 * - 专注于服务器特有的业务逻辑
 * - 与外部消息协议解耦
 * - 提供简化的服务器接口
 * - 使用统一的ConnectionTypes结构体，避免重复定义
 */
class EndpointServer : public EndpointService {
public:
	/**
	 * @brief 服务器事件处理器 - 直接使用基类EventHandler
	 */
	using EventHandler = IEndpointService::EventHandler;

public:
	explicit EndpointServer(const EndpointIdentity& config);
	~EndpointServer() override;

	// 服务器特有的客户端管理方法
	/**
	 * @brief 注册客户端
	 * @param client_id 客户端ID
	 * @param client_info 客户端信息
	 * @return 是否注册成功
	 */
	bool RegisterClient(const std::string& client_id, const EndpointIdentity& client_info);

	/**
	 * @brief 注销客户端
	 * @param client_id 客户端ID
	 * @return 是否注销成功
	 */
	bool UnregisterClient(const std::string& client_id);

	/**
	 * @brief 断开客户端连接
	 * @param client_id 客户端ID
	 * @return 是否成功断开
	 */
	bool DisconnectClient(const std::string& client_id);

	/**
	 * @brief 获取客户端信息
	 * @param client_id 客户端ID
	 * @return 客户端信息
	 */
	EndpointIdentity GetClientInfo(const std::string& client_id) const;

	/**
	 * @brief 获取所有客户端
	 * @return 客户端信息列表
	 */
	std::vector<EndpointIdentity> GetAllClients() const;

	/**
	 * @brief 获取在线客户端
	 * @return 在线客户端信息列表
	 */
	std::vector<EndpointIdentity> GetOnlineClients() const;

	/**
	 * @brief 检查是否有指定客户端
	 * @param client_id 客户端ID
	 * @return 是否存在
	 */
	bool HasClient(const std::string& client_id) const;

	/**
	 * @brief 检查客户端是否在线
	 * @param client_id 客户端ID
	 * @return 是否在线
	 */
	bool IsClientOnline(const std::string& client_id) const;
	
	/**
	 * @brief 检查客户端心跳是否正常
	 * @param client_id 客户端ID
	 * @return 心跳是否正常
	 */
	bool IsClientHeartbeatAlive(const std::string& client_id) const;
	
	/**
	 * @brief 获取客户端心跳统计信息
	 * @return 心跳统计信息
	 */
	std::string GetHeartbeatStatistics() const;

	/**
	 * @brief 发送消息给客户端
	 * @param client_id 客户端ID
	 * @param message_data 消息数据
	 * @return 是否发送成功
	 */
	bool SendToClient(const std::string& client_id, const std::vector<uint8_t>& message_data);

	/**
	 * @brief 发送请求给客户端并等待响应
	 * @param client_id 客户端ID
	 * @param request_data 请求数据
	 * @param timeout_ms 超时时间
	 * @return 响应数据
	 */
	std::vector<uint8_t> SendRequestToClient(const std::string& client_id, 
	                                      const std::vector<uint8_t>& request_data, 
	                                      uint32_t timeout_ms = 5000);

	/**
	 * @brief 广播消息给客户端
	 * @param message_data 消息数据
	 * @param client_ids 客户端ID列表（为空则广播给所有在线客户端）
	 */
	void BroadcastToClients(const std::vector<uint8_t>& message_data, 
	                      const std::vector<std::string>& client_ids = {});

	/**
	 * @brief 注册服务器事件处理器（封装用户处理器，并在内部维护客户端状态）
	 * @param handler 事件处理器
	 */
	void RegisterEventHandler(EventHandler::Ptr handler) override;

	// EndpointService接口实现（覆盖基类方法）
	bool Initialize() override;
	bool Start() override;
	void Stop() override;
	void Cleanup() override;
	
	bool SendMessage(const std::string& target_id, const std::vector<uint8_t>& message_data, uint32_t timeout_ms = 0) override;
	void BroadcastMessage(const std::vector<uint8_t>& message_data, 
	                    const std::string& target_name = "") override;
	
	// 心跳支持实现
	void EnableHeartbeat(bool enable) override;
	bool IsHeartbeatEnabled() const override;
	
	// 心跳处理纯虚函数实现
	void OnHeartbeatRequest(std::shared_ptr<ITransport> transport, const std::string& endpoint_id, 
                               uint16_t message_id, uint8_t sub_message_id, const std::vector<uint8_t>& payload) override;
	void OnHeartbeatResponse(std::shared_ptr<ITransport> transport, const std::string& endpoint_id, 
                                uint16_t message_id, uint8_t sub_message_id, const std::vector<uint8_t>& payload) override;
	
	// 心跳管理方法
	void StartHeartbeatMonitor();
	void StopHeartbeatMonitor();
	void SendHeartbeatRequest(const std::string& target_id);
	const ClientHeartbeatInfo* GetClientHeartbeatInfo(const std::string& client_id) const;
	std::unordered_map<std::string, ClientHeartbeatInfo> GetAllClientHeartbeatInfo() const;
	
	// 统计接口与时间接口复用基类

private:
	// 私有方法
	void CleanupOfflineClients();
	uint64_t GetCurrentTimestamp() const;

	// 内部事件处理器：先维护内部状态，再转发给用户处理器
	class InternalServerEventHandler : public IEndpointService::EventHandler {
	public:
		explicit InternalServerEventHandler(EndpointServer* server) : server_(server) {}

		void OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) override;
		void OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) override;
		void OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) override;

	private:
		EndpointServer* server_;
	};

private:
	// 服务器特有状态（使用统一的EndpointIdentity）
	std::unordered_map<std::string, EndpointIdentity> clients_;
	mutable std::mutex clients_mutex_;

	// 外部（用户）事件处理器
	EventHandler::Ptr user_handler_;
	
	// 心跳相关
	std::atomic<bool> heartbeat_enabled_{false};
	std::thread heartbeat_monitor_thread_;
	std::atomic<bool> heartbeat_monitor_running_{false};
	std::unordered_map<std::string, ClientHeartbeatInfo> client_heartbeat_info_;
	mutable std::mutex heartbeat_info_mutex_;
};

} // namespace perception
