#pragma once

#include "communication/endpoints/services/EndpointService.hpp"
#include <memory>
#include <functional>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>

namespace perception {

/**
 * @brief 端点客户端 - 基于端点服务的客户端实现
 * 
 * 设计原则：
 * - 继承自EndpointService基类
 * - 专注于客户端特有的业务逻辑
 * - 与外部消息协议解耦
 * - 提供简化的客户端接口
 * - 复用基类的统计信息结构体，减少重复定义
 */
class EndpointClient : public EndpointService {
public:
	/**
	 * @brief 客户端事件处理器 - 直接使用基类EventHandler
	 */
	using EventHandler = IEndpointService::EventHandler;

public:
	explicit EndpointClient(const EndpointConfig& config);
	~EndpointClient() override;

	// 客户端特有的连接方法
	/**
	 * @brief 连接到指定地址和端口的服务器
	 * @param server_address 服务器地址
	 * @param server_port 服务器端口
	 * @return 是否连接成功
	 */
	bool ConnectToServer(const std::string& server_address, uint16_t server_port);

	/**
	 * @brief 断开服务器连接
	 */
	void DisconnectFromServer();

	/**
	 * @brief 检查是否已连接到服务器
	 * @return 是否已连接
	 */
	bool IsConnectedToServer() const;

	/**
	 * @brief 获取连接状态
	 * @return 连接状态字符串
	 */
	std::string GetConnectionStatus() const;

	/**
	 * @brief 注册客户端事件处理器
	 * @param handler 事件处理器
	 */
	void RegisterEventHandler(EventHandler::Ptr handler) override;

	/**
	 * @brief 发送消息到服务器
	 * @param target_id 目标ID（可选，默认发送到连接的服务器）
	 * @param message_data 消息数据
	 * @param timeout_ms 超时时间（毫秒），0表示不等待响应
	 * @return 是否发送成功
	 */
	bool SendMessage(const std::string& target_id, const std::vector<uint8_t>& message_data, uint32_t timeout_ms = 0) override;

	/**
	 * @brief 广播消息
	 * @param message_data 消息数据
	 * @param target_name 目标名称
	 */
	void BroadcastMessage(const std::vector<uint8_t>& message_data, 
	                    const std::string& target_name = "") override;

	// EndpointService接口实现（覆盖基类方法）
	bool Initialize() override;
	bool Start() override;
	void Stop() override;
	void Cleanup() override;

private:
	// 私有方法
	void AutoReconnect();
	void SendHeartbeat();
	void StartConnectionMonitor();
	void StopConnectionMonitor();
	void StartHeartbeat();
	void StopHeartbeat();
	void ConnectionMonitorLoop();
	void HeartbeatLoop();
	bool TryConnectToServer();
	void ResetReconnectAttempts();
	bool IsHeartbeatMessage(const std::vector<uint8_t>& message_data) const;

	// 内部事件处理器：先维护内部状态，再转发给用户处理器
	class InternalClientEventHandler : public IEndpointService::EventHandler {
	public:
		explicit InternalClientEventHandler(EndpointClient* client) : client_(client) {}

		void OnMessageReceived(const std::string& endpoint_id, const std::vector<uint8_t>& message_data) override;
		void OnConnectionChanged(const std::string& endpoint_id, bool connected, const ConnectionInfo& connection_info) override;
		void OnError(const std::string& endpoint_id, uint16_t error_code, const std::string& error_message) override;

	private:
		EndpointClient* client_;
	};

private:
	// 客户端特有状态（复用基类的状态管理）
	std::atomic<bool> connected_{false};
	std::string connected_server_id_;
	
	// 重连相关
	std::atomic<bool> reconnect_enabled_{false};
	std::atomic<uint64_t> last_connection_attempt_{0};
	std::atomic<uint64_t> last_heartbeat_{0};
	
	// 监控线程
	std::thread connection_monitor_thread_;
	std::thread heartbeat_thread_;
	std::atomic<bool> monitor_running_{false};

	// 外部（用户）事件处理器
	EventHandler::Ptr user_handler_;

	// 运行时连接目标（不放在配置中）
	std::string target_server_address_;
	uint16_t target_server_port_{0};
};

} // namespace perception
