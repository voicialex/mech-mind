#include "CameraManager.hpp"
#include "utils/Logger.hpp"

int main()
{
    // 初始化日志系统
    Logger::getInstance().setLevel(Logger::Level::INFO);
    LOG_INFO_STREAM << "Application started with stream operator";
    
    CameraManager cameraManager;
    cameraManager.Init();
    cameraManager.Connect();
    cameraManager.Start();
    cameraManager.Stop();
    
    LOG_INFO_STREAM << "Application finished with stream operator";
    return 0;
}