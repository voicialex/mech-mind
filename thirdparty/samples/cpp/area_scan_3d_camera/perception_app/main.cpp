#include "CameraManager.hpp"
#include "utils/Logger.hpp"
#include "ConfigHelper.hpp"

int main()
{
    // 初始化日志系统
    Logger::getInstance().setLevel(Logger::Level::INFO);
    LOG_INFO_STREAM << "Application started with stream operator";
    
    // 加载配置文件
    ConfigHelper& config = ConfigHelper::getInstance();
    if (!config.loadConfigFromJson()) {
        LOG_INFO_STREAM << "use default config";
    }
    
    // 打印当前配置信息
    // config.printConfig();
    
    CameraManager cameraManager;
    cameraManager.Init();
    cameraManager.Connect();
    cameraManager.Start();
    cameraManager.Stop();
    
    LOG_INFO_STREAM << "Application finished with stream operator";
    return 0;
}