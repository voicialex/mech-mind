#pragma once

#include "EndpointService.hpp"
#include <string>
#include <sstream>
#include <iomanip>

namespace perception {

// 前向声明
class EndpointServer;

class ServiceInspector {
public:
	/**
	 * @brief 生成端点服务的完整统计信息
	 * @param svc 端点服务实例
	 * @return 格式化的统计信息字符串
	 */
	static std::string DumpText(const EndpointService& svc);
	
	/**
	 * @brief 生成心跳统计信息
	 * @param svc 端点服务实例
	 * @return 格式化的心跳统计信息字符串
	 */
	static std::string GetHeartbeatStatistics(const EndpointService& svc);
	
	/**
	 * @brief 检查客户端心跳是否正常
	 * @param svc 端点服务实例
	 * @param client_id 客户端ID
	 * @return 心跳是否正常
	 */
	static bool IsClientHeartbeatAlive(const EndpointService& svc, const std::string& client_id);
	
	/**
	 * @brief 获取客户端心跳信息
	 * @param svc 端点服务实例
	 * @param client_id 客户端ID
	 * @return 心跳信息指针，如果不存在则返回nullptr
	 */
	static const void* GetClientHeartbeatInfo(const EndpointService& svc, const std::string& client_id);
};

} // namespace perception


