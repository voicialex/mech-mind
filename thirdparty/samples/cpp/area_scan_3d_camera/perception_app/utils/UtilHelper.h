#include <iostream>
#include <vector>
#include "area_scan_3d_camera/CameraProperties.h"
#include "ErrorStatus.h"
#include "CommonTypes.h"
#include "area_scan_3d_camera/Camera.h"
#include "utils/Logger.hpp"

inline void printCameraInfo(const mmind::eye::CameraInfo& cameraInfo)
{
    LOG_INFO_STREAM << ".............................";
    LOG_INFO_STREAM << "Model:                        " << cameraInfo.model;
    LOG_INFO_STREAM << "Device name:                  " << cameraInfo.deviceName;
    LOG_INFO_STREAM << "Serial number:                " << cameraInfo.serialNumber;
    LOG_INFO_STREAM << "IP address:                   " << cameraInfo.ipAddress;
    LOG_INFO_STREAM << "Subnet mask:                  " << cameraInfo.subnetMask;
    LOG_INFO_STREAM << "IP address assignment method: "
              << mmind::eye::ipAssignmentMethodToString(cameraInfo.ipAssignmentMethod);
    LOG_INFO_STREAM << "Hardware version:             "
              << "V" << cameraInfo.hardwareVersion.toString();
    LOG_INFO_STREAM << "Firmware version:             "
              << "V" << cameraInfo.firmwareVersion.toString();
    LOG_INFO_STREAM << ".............................";
    LOG_INFO_STREAM << std::endl;
}



inline bool FindAndConnect(mmind::eye::Camera& device)
{
    std::cout << "Looking for available cameras..." << std::endl;
    std::vector<mmind::eye::CameraInfo> deviceInfoList = mmind::eye::Camera::discoverCameras();

    if (deviceInfoList.empty()) {
        LOG_INFO_STREAM << "No cameras are available.";
        return false;
    }

    for (int i = 0; i < deviceInfoList.size(); i++) {
        LOG_INFO_STREAM << "Mech-Eye device index: " << i;
        printCameraInfo(deviceInfoList[i]);
    }

    // 连接重试逻辑：最多重试3次，每次间隔2秒
    int maxRetries = 3;
    mmind::eye::ErrorStatus status;
    while(maxRetries > 0) {
        status = device.connect(deviceInfoList[0]);
        if (!status.isOK()) {
            showError(status);
            maxRetries--;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        } else {
            LOG_INFO_STREAM << "Successfully connected to the camera.";
            break;
        }
    }
    return status.isOK();
}