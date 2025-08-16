/*******************************************************************************
 *BSD 3-Clause License
 *
 *Copyright (c) 2016-2025, Mech-Mind Robotics Technologies Co., Ltd.
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *
 *1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 *3. Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#pragma once
#include <iostream>
#include <set>
#include <utility>
#include <regex>
#ifdef WIN32
#include <windows.h>
#endif
#include "CameraProperties.h"
#include "ErrorStatus.h"
#include "CommonTypes.h"
#include "Camera.h"

inline void printCameraInfo(const mmind::eye::CameraInfo& cameraInfo)
{
    std::cout << "............................." << std::endl;
    std::cout << "Model:                        " << cameraInfo.model << std::endl;
#ifdef WIN32
    const auto consoleCP = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::cout << "Device name:                  " << cameraInfo.deviceName << std::endl;
#ifdef WIN32
    SetConsoleOutputCP(consoleCP);
#endif
    std::cout << "Serial number:                " << cameraInfo.serialNumber << std::endl;
    std::cout << "IP address:                   " << cameraInfo.ipAddress << std::endl;
    std::cout << "Subnet mask:                  " << cameraInfo.subnetMask << std::endl;
    std::cout << "IP address assignment method: "
              << mmind::eye::ipAssignmentMethodToString(cameraInfo.ipAssignmentMethod) << std::endl;
    std::cout << "Hardware version:             "
              << "V" << cameraInfo.hardwareVersion.toString() << std::endl;
    std::cout << "Firmware version:             "
              << "V" << cameraInfo.firmwareVersion.toString() << std::endl;
    std::cout << "............................." << std::endl;
    std::cout << std::endl;
}

inline void printCameraStatus(const mmind::eye::CameraStatus& cameraStatus)
{
    std::cout << ".....Camera temperatures....." << std::endl;
    std::cout << "CPU:       " << cameraStatus.temperature.cpuTemperature << "°C" << std::endl;
    std::cout << "Projector: " << cameraStatus.temperature.projectorTemperature << "°C"
              << std::endl;
    std::cout << "............................" << std::endl;

    std::cout << std::endl;
}

inline void printCameraResolutions(const mmind::eye::CameraResolutions& cameraResolutions)

{
    std::cout << ".....Image resolutions....." << std::endl;
    std::cout << "2D image (texture): " << cameraResolutions.texture.width << " (width) × "
              << cameraResolutions.texture.height << " (height)" << std::endl;
    std::cout << "Depth map:          " << cameraResolutions.depth.width << " (width) × "
              << cameraResolutions.depth.height << " (height)" << std::endl;
}

inline void printCameraMatrix(const std::string& title,
                              const mmind::eye::CameraMatrix& cameraMatrix)
{
    std::cout << title << ": " << std::endl
              << "    [" << cameraMatrix.fx << ", " << 0 << ", " << cameraMatrix.cx << "]"

              << std::endl
              << "    [" << 0 << ", " << cameraMatrix.fy << ", " << cameraMatrix.cy << "]"

              << std::endl
              << "    [" << 0 << ", " << 0 << ", " << 1 << "]" << std::endl;
    std::cout << std::endl;
}

inline void printCameraDistCoeffs(const std::string& title,
                                  const mmind::eye::CameraDistortion& distCoeffs)
{
    std::cout << title << ": " << std::endl
              << "    k1: " << distCoeffs.k1 << ", k2: " << distCoeffs.k2
              << ", p1: " << distCoeffs.p1 << ", p2: " << distCoeffs.p2 << ", k3: " << distCoeffs.k3
              << std::endl;
    std::cout << std::endl;
}

inline void printTransform(const std::string& title, const mmind::eye::Transformation& transform)
{
    std::cout << "Rotation: " << title << ": " << std::endl;
    for (int i = 0; i < 3; i++) {
        std::cout << "    [";
        for (int j = 0; j < 3; j++) {
            std::cout << transform.rotation[i][j];
            if (j != 2)
                std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Translation " << title << ": " << std::endl;
    std::cout << "    X: " << transform.translation[0] << "mm, Y: " << transform.translation[1]
              << "mm, Z: " << transform.translation[2] << "mm" << std::endl;
    std::cout << std::endl;
}

inline void printCameraIntrinsics(const mmind::eye::CameraIntrinsics& intrinsics)
{
    printCameraMatrix("Texture 2D camera matrix", intrinsics.texture.cameraMatrix);
    printCameraDistCoeffs("Texture 2D camera distortion coefficients",
                          intrinsics.texture.cameraDistortion);

    printCameraMatrix("Depth 2D camera matrix", intrinsics.depth.cameraMatrix);
    printCameraDistCoeffs("Depth 2D camera distortion coefficients",
                          intrinsics.depth.cameraDistortion);

    printTransform("Transformation from depth 2D camera to texture 2D camera",
                   intrinsics.depthToTexture);
}

inline bool findAndConnect(mmind::eye::Camera& device)
{
    std::cout << "Looking for available cameras..." << std::endl;
    std::vector<mmind::eye::CameraInfo> deviceInfoList = mmind::eye::Camera::discoverCameras();

    if (deviceInfoList.empty()) {
        std::cout << "No cameras are available." << std::endl;
        return false;
    }

    for (int i = 0; i < deviceInfoList.size(); i++) {
        std::cout << "Mech-Eye device index: " << i << std::endl;
        printCameraInfo(deviceInfoList[i]);
    }

    std::cout << "Enter the index of the device to which you want to connect: ";
    unsigned inputIndex = 0;

    while (true) {
        std::string str;
        std::cin >> str;
        if (std::regex_match(str.begin(), str.end(), std::regex{"[0-9]+"}) &&
            atoi(str.c_str()) < deviceInfoList.size()) {
            inputIndex = atoi(str.c_str());
            break;
        }
        std::cout << "The entered index is invalid. Please enter the device index again: ";
    }

    mmind::eye::ErrorStatus status;
    status = device.connect(deviceInfoList[inputIndex]);

    if (!status.isOK()) {
        showError(status);
        return false;
    }

    std::cout << "Successfully connected to the camera." << std::endl;
    return true;
}

inline std::vector<mmind::eye::Camera> findAndConnectMultiCamera()
{
    std::cout << "Looking for available cameras..." << std::endl;
    std::vector<mmind::eye::CameraInfo> cameraInfoList = mmind::eye::Camera::discoverCameras();

    if (cameraInfoList.empty()) {
        std::cout << "No cameras are available." << std::endl;
        return {};
    }

    for (int i = 0; i < cameraInfoList.size(); i++) {
        std::cout << "Mech-Eye device index: " << i << std::endl;
        printCameraInfo(cameraInfoList[i]);
    }

    std::string str;
    std::set<unsigned> indices;

    while (true) {
        std::cout << "Enter the indices of the devices to which you want to connect: " << std::endl;
        std::cout << "Enter the character \"c\" at the end of all the indices" << std::endl;

        std::cin >> str;
        if (str == "c")
            break;
        if (std::regex_match(str.begin(), str.end(), std::regex{"[0-9]+"}) &&
            atoi(str.c_str()) < cameraInfoList.size())
            indices.insert(atoi(str.c_str()));
        else
            std::cout << "The entered indices are invalid. Please enter the device indices again: ";
    }

    std::vector<mmind::eye::Camera> cameraList{};

    auto iter = indices.cbegin();
    for (int i = 0; i < indices.size(); ++i, ++iter) {
        mmind::eye::Camera camera;
        auto status = camera.connect(cameraInfoList[*iter]);
        if (status.isOK())
            cameraList.push_back(camera);
        else
            showError(status);
    }

    return cameraList;
}

inline bool confirmCapture3D()
{
    std::cout
        << "Do you want the camera to capture 3D data? Enter \"y\" to confirm or \"n\" to cancel: "
        << std::endl;
    while (true) {
        std::string confirmStr;
        std::cin >> confirmStr;
        if (confirmStr == "y") {
            return true;
        } else if (confirmStr == "n") {
            std::cout << "The capture command was canceled." << std::endl;
            return false;
        } else {
            std::cout << "The entered character was invalid. Please enter \"y\" to confirm or "
                         "\"n\" to cancel:"
                      << std::endl;
        }
    }
}
