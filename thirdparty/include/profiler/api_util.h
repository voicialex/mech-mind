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
#include <iomanip>
#include <iostream>
#include <regex>
#include <set>
#include <utility>
#ifdef WIN32
#include <windows.h>
#endif
#include "ErrorStatus.h"
#include "CommonTypes.h"
#include "profiler/parameters/ScanParameters.h"
#include "Profiler.h"
#include "ProfilerInfo.h"

/**
 * @brief Prints the data in the ProfilerInfo object.
 */
inline void printProfilerInfo(const mmind::eye::ProfilerInfo& profilerInfo)
{
    std::cout << "........................................." << std::endl;
    std::cout << "Model:                         " << profilerInfo.model << std::endl;
#ifdef WIN32
    const auto consoleCP = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::cout << "Device name:                   " << profilerInfo.deviceName << std::endl;
#ifdef WIN32
    SetConsoleOutputCP(consoleCP);
#endif
    std::cout << "Controller serial number:      " << profilerInfo.controllerSN << std::endl;
    std::cout << "Sensor head serial number:     " << profilerInfo.sensorSN << std::endl;
    std::cout << "IP address:                    " << profilerInfo.ipAddress << std::endl;
    std::cout << "Subnet mask:                   " << profilerInfo.subnetMask << std::endl;
    std::cout << "IP address assignment method:  "
              << mmind::eye::ipAssignmentMethodToString(profilerInfo.ipAssignmentMethod)
              << std::endl;
    std::cout << "Hardware version:              "
              << "V" << profilerInfo.hardwareVersion.toString() << std::endl;
    std::cout << "Firmware version:              "
              << "V" << profilerInfo.firmwareVersion.toString() << std::endl;
    std::cout << "........................................." << std::endl;
    std::cout << std::endl;
}

inline void printProfilerStatus(const mmind::eye::ProfilerStatus& profilerStatus)
{
    std::cout << ".....Profiler temperatures....." << std::endl;
    std::cout << "Controller CPU: " << std::setprecision(4)
              << profilerStatus.temperature.controllerCpuTemperature << "°C" << std::endl;
    std::cout << "Sensor CPU:     " << std::setprecision(4)
              << profilerStatus.temperature.sensorCpuTemperature << "°C" << std::endl;
    std::cout << "..............................." << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Discovers all available laser profilers and allows the user to connect to a laser profiler
 * by inputting the device index.
 */
inline bool findAndConnect(mmind::eye::Profiler& profiler)
{
    std::cout << "Looking for available profilers..." << std::endl;
    std::vector<mmind::eye::ProfilerInfo> profilerInfoList =
        mmind::eye::Profiler::discoverProfilers();

    if (profilerInfoList.empty()) {
        std::cout << "No profilers are available." << std::endl;
        return false;
    }

    for (int i = 0; i < profilerInfoList.size(); i++) {
        std::cout << "Mech-Eye device index: " << i << std::endl;
        printProfilerInfo(profilerInfoList[i]);
    }

    std::cout << "Enter the index of the device to which you want to connect: ";
    unsigned inputIndex = 0;

    while (true) {
        std::string str;
        std::cin >> str;
        if (std::regex_match(str.begin(), str.end(), std::regex{"[0-9]+"}) &&
            atoi(str.c_str()) < profilerInfoList.size()) {
            inputIndex = atoi(str.c_str());
            break;
        }
        std::cout << "The entered index is invalid. Please enter the device index again: ";
    }

    mmind::eye::ErrorStatus status;
    status = profiler.connect(profilerInfoList[inputIndex]);

    if (!status.isOK()) {
        showError(status);
        return false;
    }

    std::cout << "Successfully connected to the profiler." << std::endl;
    return true;
}

inline std::vector<mmind::eye::Profiler> findAndConnectMultiProfiler()
{
    std::cout << "Looking for available profilers..." << std::endl;
    std::vector<mmind::eye::ProfilerInfo> profilerInfoList =
        mmind::eye::Profiler::discoverProfilers();

    if (profilerInfoList.empty()) {
        std::cout << "No profilers are available." << std::endl;
        return {};
    }

    for (int i = 0; i < profilerInfoList.size(); i++) {
        std::cout << "Mech-Eye device index: " << i << std::endl;
        printProfilerInfo(profilerInfoList[i]);
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
            atoi(str.c_str()) < profilerInfoList.size())
            indices.insert(atoi(str.c_str()));
        else
            std::cout << "The entered indices are invalid. Please enter the device indices again: ";
    }

    std::vector<mmind::eye::Profiler> profilerList{};

    auto iter = indices.cbegin();
    for (int i = 0; i < indices.size(); ++i, ++iter) {
        mmind::eye::Profiler profiler;
        auto status = profiler.connect(profilerInfoList[*iter]);
        if (status.isOK())
            profilerList.push_back(profiler);
        else
            showError(status);
    }

    return profilerList;
}

inline bool confirmCapture()
{
    std::cout
        << "Do you want the profiler to capture image? Enter \"y\" to confirm or \"n\" to cancel: "
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
            std::cout << "The entered character was invalid. Please enter \"y\" ot confirm or "
                         "\"n\" to cancel:"
                      << std::endl;
        }
    }
}

inline void savePointCloud(const mmind::eye::ProfileBatch& batch,
                           const mmind::eye::UserSet& userSet, bool savePLY = true,
                           bool saveCSV = true, bool isOrganized = false)
{
    if (batch.isEmpty())
        return;

    // Get the X-axis resolution
    double xResolution{};
    auto status = userSet.getFloatValue(mmind::eye::point_cloud_resolutions::XAxisResolution::name,
                                        xResolution);
    if (!status.isOK()) {
        showError(status);
        return;
    }

    // Get the Y resolution
    double yResolution{};
    status =
        userSet.getFloatValue(mmind::eye::point_cloud_resolutions::YResolution::name, yResolution);
    if (!status.isOK()) {
        showError(status);
        return;
    }
    // // Uncomment the following lines for custom Y Unit
    // // Prompt to enter the desired encoder resolution, which is the travel distance corresponding
    // // to
    // // one quadrature signal.
    // std::cout << "Please enter the desired encoder resolution (integer, unit: μm, min: "
    //  "1, max: 65535): ";
    // while (true) {
    //     std::string str;
    //     std::cin >> str;
    //     if (std::regex_match(str.begin(), str.end(), std::regex{"[0-9]+"})) {
    //         yResolution = atoi(str.c_str());
    //         break;
    //     }
    //     std::cout << "Input invalid! Please enter the desired encoder resolution (integer, unit:
    //     "
    //                  "μm, min: 1, max: 65535): ";
    // }

    int lineScanTriggerSource{};
    status = userSet.getEnumValue(mmind::eye::trigger_settings::LineScanTriggerSource::name,
                                  lineScanTriggerSource);
    if (!status.isOK()) {
        showError(status);
        return;
    }

    bool useEncoderValues =
        lineScanTriggerSource ==
        static_cast<int>(mmind::eye::trigger_settings::LineScanTriggerSource::Value::Encoder);

    int triggerInterval{};
    status = userSet.getIntValue(mmind::eye::trigger_settings::EncoderTriggerInterval::name,
                                 triggerInterval);
    if (!status.isOK()) {
        showError(status);
        return;
    }

    std::cout << "Save the point cloud." << std::endl;
    if (saveCSV)
        showError(batch.saveUntexturedPointCloud(
            xResolution, yResolution, useEncoderValues, triggerInterval,
            mmind::eye::FileFormat::CSV, "PointCloud.csv", mmind::eye::CoordinateUnit::Millimeter,
            isOrganized));
    if (savePLY)
        showError(batch.saveUntexturedPointCloud(
            xResolution, yResolution, useEncoderValues, triggerInterval,
            mmind::eye::FileFormat::PLY, "PointCloud.ply", mmind::eye::CoordinateUnit::Millimeter,
            isOrganized));
}
