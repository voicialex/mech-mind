#include "camera/CameraManager.hpp"
#include "Logger.hpp"
#include "configure/ConfigHelper.hpp"
#include "InferenceInterface.hpp"
#include "ExampleInference.hpp"

int main() {
  // Initialize logging system
  Logger::getInstance().setLevel(Logger::Level::INFO);
  LOG_INFO_STREAM << "Application started with stream operator";

  // Load configuration file
  ConfigHelper &config = ConfigHelper::getInstance();
  if (!config.loadConfigFromJson()) {
    LOG_INFO_STREAM << "use default config";
  }

  // Print current configuration
  // config.printConfig();

  // Create and register inference algorithm
  auto example_inference = std::make_shared<ExampleInference>();
  if (!InferenceManager::getInstance().RegisterInference(example_inference)) {
    LOG_ERROR_STREAM << "Failed to register inference algorithm";
    return -1;
  }

  CameraManager cameraManager;
  cameraManager.Init();
  cameraManager.Connect();

  cameraManager.Start();
  cameraManager.Stop();

  LOG_INFO_STREAM << "Application finished with stream operator";
  return 0;
}