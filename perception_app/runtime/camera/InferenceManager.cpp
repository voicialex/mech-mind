#include "InferenceInterface.hpp"
#include "Logger.hpp"

InferenceManager &InferenceManager::getInstance() {
  static InferenceManager instance;
  return instance;
}

bool InferenceManager::RegisterInference(std::shared_ptr<InferenceInterface> inference_interface) {
  if (!inference_interface) {
    LOG_ERROR_STREAM << "Failed to register inference: null pointer";
    return false;
  }

  inference_interface_ = inference_interface;
  LOG_INFO_STREAM << "Successfully registered inference: " << inference_interface_->GetAlgorithmName();
  return true;
}

bool InferenceManager::InitializeInference(const std::string &config_path) {
  if (!inference_interface_) {
    LOG_ERROR_STREAM << "Failed to initialize inference: no inference interface registered";
    return false;
  }

  if (inference_interface_->Initialize(config_path)) {
    is_initialized_ = true;
    LOG_INFO_STREAM << "Successfully initialized inference: " << inference_interface_->GetAlgorithmName();
    return true;
  } else {
    LOG_ERROR_STREAM << "Failed to initialize inference: " << inference_interface_->GetAlgorithmName();
    return false;
  }
}

bool InferenceManager::Process(const FrameSet &frame_set) {
  if (!is_initialized_ || !inference_interface_) {
    LOG_WARNING_STREAM << "Cannot process frame: inference not initialized";
    return false;
  }

  if (inference_interface_->Process(frame_set)) {
    LOG_DEBUG_STREAM << "Successfully processed frame with inference: " << inference_interface_->GetAlgorithmName();
    return true;
  } else {
    LOG_ERROR_STREAM << "Failed to process frame with inference: " << inference_interface_->GetAlgorithmName();
    return false;
  }
}

std::string InferenceManager::GetResult() const {
  if (!is_initialized_ || !inference_interface_) {
    return "No inference result available";
  }
  return inference_interface_->GetResult();
}

void InferenceManager::Cleanup() {
  if (inference_interface_) {
    inference_interface_->Cleanup();
    LOG_INFO_STREAM << "Cleaned up inference: " << inference_interface_->GetAlgorithmName();
  }
  is_initialized_ = false;
}

bool InferenceManager::IsInitialized() const {
  return is_initialized_ && inference_interface_ && inference_interface_->IsInitialized();
}

std::string InferenceManager::GetCurrentAlgorithmName() const {
  if (inference_interface_) {
    return inference_interface_->GetAlgorithmName();
  }
  return "No algorithm registered";
}
