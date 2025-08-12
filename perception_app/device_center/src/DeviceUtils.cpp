#include "../include/DeviceTypes.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>

using namespace device_center;

namespace device_center {

std::string DeviceUtils::StatusToString(DeviceStatus status) {
  switch (status) {
    case DeviceStatus::Offline:
      return "Offline";
    case DeviceStatus::Online:
      return "Online";
    case DeviceStatus::Busy:
      return "Busy";
    case DeviceStatus::Error:
      return "Error";
    case DeviceStatus::Maintenance:
      return "Maintenance";
    default:
      return "Unknown";
  }
}

DeviceStatus DeviceUtils::StringToStatus(const std::string &status_str) {
  if (status_str == "Offline") return DeviceStatus::Offline;
  if (status_str == "Online") return DeviceStatus::Online;
  if (status_str == "Busy") return DeviceStatus::Busy;
  if (status_str == "Error") return DeviceStatus::Error;
  if (status_str == "Maintenance") return DeviceStatus::Maintenance;
  return DeviceStatus::Offline;
}

std::string DeviceUtils::TypeToString(DeviceType type) {
  switch (type) {
    case DeviceType::Camera:
      return "Camera";
    case DeviceType::Sensor:
      return "Sensor";
    case DeviceType::Robot:
      return "Robot";
    case DeviceType::Controller:
      return "Controller";
    case DeviceType::Actuator:
      return "Actuator";
    default:
      return "Unknown";
  }
}

DeviceType DeviceUtils::StringToType(const std::string &type_str) {
  if (type_str == "Camera") return DeviceType::Camera;
  if (type_str == "Sensor") return DeviceType::Sensor;
  if (type_str == "Robot") return DeviceType::Robot;
  if (type_str == "Controller") return DeviceType::Controller;
  if (type_str == "Actuator") return DeviceType::Actuator;
  return DeviceType::Unknown;
}

std::string DeviceUtils::CapabilityToString(DeviceCapability capability) {
  switch (capability) {
    case DeviceCapability::Read:
      return "Read";
    case DeviceCapability::Write:
      return "Write";
    case DeviceCapability::Control:
      return "Control";
    case DeviceCapability::Configure:
      return "Configure";
    case DeviceCapability::Status:
      return "Status";
    case DeviceCapability::Calibrate:
      return "Calibrate";
    case DeviceCapability::Capture:
      return "Capture";
    case DeviceCapability::Process:
      return "Process";
    default:
      return "Unknown";
  }
}

DeviceCapability DeviceUtils::StringToCapability(const std::string &capability_str) {
  if (capability_str == "Read") return DeviceCapability::Read;
  if (capability_str == "Write") return DeviceCapability::Write;
  if (capability_str == "Control") return DeviceCapability::Control;
  if (capability_str == "Configure") return DeviceCapability::Configure;
  if (capability_str == "Status") return DeviceCapability::Status;
  if (capability_str == "Calibrate") return DeviceCapability::Calibrate;
  if (capability_str == "Capture") return DeviceCapability::Capture;
  if (capability_str == "Process") return DeviceCapability::Process;
  return DeviceCapability::Read;
}

std::string DeviceUtils::GenerateDeviceId(DeviceType device_type, const std::string &device_name) {
  std::ostringstream oss;
  oss << static_cast<int>(device_type) << "-" << device_name << "-" << GetCurrentTimestamp();
  return oss.str();
}

bool DeviceUtils::ValidateDeviceId(const std::string &device_id) { return !device_id.empty(); }

uint64_t DeviceUtils::GetCurrentTimestamp() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

std::string DeviceUtils::TimestampToString(uint64_t timestamp) {
  std::ostringstream oss;
  oss << timestamp;
  return oss.str();
}

uint64_t DeviceUtils::StringToTimestamp(const std::string &time_str) {
  try {
    return static_cast<uint64_t>(std::stoull(time_str));
  } catch (...) {
    return 0ULL;
  }
}

}  // namespace device_center
