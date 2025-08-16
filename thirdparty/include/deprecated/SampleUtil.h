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
#include "MechEyeApi.h"
#include "MechEyeLNXApi.h"

inline void showError(const mmind::api::ErrorStatus& status)
{
    if (status.isOK())
        return;
    std::cout << "Error Code : " << status.errorCode
              << ", Error Description: " << status.errorDescription << std::endl;
}

inline void printDeviceInfo(const mmind::api::MechEyeDeviceInfo& deviceInfo)
{
    std::cout << "............................" << std::endl;
    std::cout << "Camera Model Name: " << deviceInfo.model << std::endl;
    std::cout << "Camera ID:         " << deviceInfo.id << std::endl;
    std::cout << "Camera IP Address: " << deviceInfo.ipAddress << std::endl;
    std::cout << "Hardware Version:  "
              << "V" << deviceInfo.hardwareVersion << std::endl;
    std::cout << "Firmware Version:  "
              << "V" << deviceInfo.firmwareVersion << std::endl;
    std::cout << "............................" << std::endl;
    std::cout << std::endl;
}

inline void printDeviceTemperature(const mmind::api::DeviceTemperature& deviceTemperature)
{
    std::cout << ".....Device Temperature....." << std::endl;
    std::cout << "CPU :               " << deviceTemperature.cpuTemperature << "°C" << std::endl;
    std::cout << "Projector Module:   " << deviceTemperature.projectorModuleTemperature << "°C"
              << std::endl;
    std::cout << "............................" << std::endl;

    std::cout << std::endl;
}

inline bool isNumber(const std::string& str)
{
    for (char it : str) {
        if (it < '0' || it > '9')
            return false;
    }
    return true;
}

inline void printDeviceResolution(const mmind::api::DeviceResolution& deviceResolution)
{
    std::cout << "Color Map size : (width : " << deviceResolution.colorMapWidth
              << ", height : " << deviceResolution.colorMapHeight << ")." << std::endl;
    std::cout << "Depth Map size : (width : " << deviceResolution.depthMapWidth
              << ", height : " << deviceResolution.depthMapHeight << ")." << std::endl;
}

inline void printCameraMatrix(const std::string& title, const double* cameraMatrix)
{
    std::cout << title << ": " << std::endl
              << "    [" << cameraMatrix[0] << ", " << 0 << ", " << cameraMatrix[2] << "]"

              << std::endl
              << "    [" << 0 << ", " << cameraMatrix[1] << ", " << cameraMatrix[3] << "]"

              << std::endl
              << "    [" << 0 << ", " << 0 << ", " << 1 << "]" << std::endl;
    std::cout << std::endl;
}

inline void printCameraDistCoeffs(const std::string& title, const double* distCoeffs)
{
    std::cout << title << ": " << std::endl
              << "    k1: " << distCoeffs[0] << ", k2: " << distCoeffs[1]
              << ", p1: " << distCoeffs[2] << ", p2: " << distCoeffs[3] << ", k3: " << distCoeffs[4]
              << std::endl;
    std::cout << std::endl;
}

inline void printTransform(const std::string& title, const mmind::api::Pose& pose)
{
    std::cout << "Rotation: " << title << ": " << std::endl;
    for (int i = 0; i < 3; i++) {
        std::cout << "    [";
        for (int j = 0; j < 3; j++) {
            std::cout << pose.rotation[i][j];
            if (j != 2)
                std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Translation " << title << ": " << std::endl;
    std::cout << "    X: " << pose.translation[0] << "mm, Y: " << pose.translation[1]
              << "mm, Z: " << pose.translation[2] << "mm" << std::endl;
    std::cout << std::endl;
}

inline void printCalibParams(const mmind::api::DeviceIntri& deviceIntri)
{
    printCameraMatrix("Texture Camera Matrix", deviceIntri.textureCameraIntri.cameraMatrix);
    printCameraDistCoeffs("Texture Camera Distortion Coefficients",
                          deviceIntri.textureCameraIntri.distortion);

    printCameraMatrix("Depth Camera Matrix", deviceIntri.depthCameraIntri.cameraMatrix);
    printCameraDistCoeffs("Depth Camera Distortion Coefficients",
                          deviceIntri.depthCameraIntri.distortion);

    printTransform("from Depth Camera to Texture Camera", deviceIntri.depthToTexture);
}

inline bool findAndConnect(mmind::api::MechEyeDevice& device)
{
    std::cout << "Find Mech-Eye Industrial 3D Cameras..." << std::endl;
    std::vector<mmind::api::MechEyeDeviceInfo> deviceInfoList =
        mmind::api::MechEyeDevice::enumerateMechEyeDeviceList();

    if (deviceInfoList.empty()) {
        std::cout << "No Mech-Eye Industrial 3D Cameras found." << std::endl;
        return false;
    }

    for (int i = 0; i < deviceInfoList.size(); i++) {
        std::cout << "Mech-Eye device index : " << i << std::endl;
        printDeviceInfo(deviceInfoList[i]);
    }

    std::cout << "Please enter the device index you want to connect: ";
    unsigned inputIndex = 0;

    while (true) {
        std::string str;
        std::cin >> str;
        if (isNumber(str) && atoi(str.c_str()) < deviceInfoList.size()) {
            inputIndex = atoi(str.c_str());
            break;
        }
        std::cout << "Input invalid! Please enter the device index you want to connect: ";
    }

    mmind::api::ErrorStatus status;
    status = device.connect(deviceInfoList[inputIndex]);

    if (!status.isOK()) {
        showError(status);
        return false;
    }

    std::cout << "Connect Mech-Eye Industrial 3D Camera Successfully." << std::endl;
    return true;
}

inline bool findAndConnect(mmind::api::lnxapi::MechEyeDevice& device)
{
    std::cout << "Find Mech-Eye device..." << std::endl;
    std::vector<mmind::api::MechEyeDeviceInfo> deviceInfoList =
        mmind::api::lnxapi::MechEyeDevice::enumerateMechEyeDeviceList();

    std::vector<mmind::api::MechEyeDeviceInfo> lnxInfos;

    for (const auto& info : deviceInfoList) {
        if (info.model == "Mech-Eye LNX 8030")
            lnxInfos.emplace_back(info);
    }

    if (lnxInfos.empty()) {
        std::cout << "No Mech-Eye device found." << std::endl;
        return false;
    }

    for (int i = 0; i < lnxInfos.size(); i++) {
        std::cout << "Mech-Eye LNX device index : " << i << std::endl;
        printDeviceInfo(lnxInfos[i]);
    }

    std::cout << "Please enter the device index you want to connect: ";
    unsigned inputIndex = 0;

    while (true) {
        std::string str;
        std::cin >> str;
        if (isNumber(str) && atoi(str.c_str()) < lnxInfos.size()) {
            inputIndex = atoi(str.c_str());
            break;
        }
        std::cout << "Input invalid! Please enter the device index you want to connect: ";
    }

    mmind::api::ErrorStatus status;
    status = device.connect(lnxInfos[inputIndex]);

    if (!status.isOK()) {
        showError(status);
        return false;
    }

    std::cout << "Connect Mech-Eye Successfully." << std::endl;
    return true;
}

inline std::pair<mmind::api::MechEyeDevice*, int> findAndConnectMulti()
{
    std::cout << "Find Mech-Eye Industrial 3D Cameras..." << std::endl;
    std::vector<mmind::api::MechEyeDeviceInfo> deviceInfoList =
        mmind::api::MechEyeDevice::enumerateMechEyeDeviceList();

    if (deviceInfoList.empty()) {
        std::cout << "No Mech-Eye Industrial 3D Cameras found." << std::endl;
        return std::make_pair(nullptr, 0);
    }

    for (int i = 0; i < deviceInfoList.size(); i++) {
        std::cout << "Mech-Eye device index : " << i << std::endl;
        printDeviceInfo(deviceInfoList[i]);
    }

    std::string str;
    std::set<unsigned> indices;

    while (true) {
        std::cout << "Please enter the device index you want to connect: " << std::endl;
        std::cout << "Enter a c to terminate adding devices" << std::endl;

        std::cin >> str;
        if (str == "c")
            break;
        if (isNumber(str) && atoi(str.c_str()) < deviceInfoList.size())
            indices.emplace(atoi(str.c_str()));
        else
            std::cout << "Input invalid! Please enter the device index you want to connect: ";
    }

    auto* devices = new mmind::api::MechEyeDevice[indices.size()];

    auto it = indices.begin();
    for (int i = 0; i < indices.size() && it != indices.end(); ++i, ++it) {
        showError(devices[i].connect(deviceInfoList[*it]));
    }

    return std::make_pair(devices, static_cast<int>(indices.size()));
}
