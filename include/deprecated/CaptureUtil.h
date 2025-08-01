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
#include "MechEyeApi.h"
#include "OpenCVUtil.h"
#include "PclUtil.h"

inline void capture(mmind::api::MechEyeDevice& device, std::string suffix)
{
    const std::string colorFile = suffix.empty() ? "ColorMap.png" : "ColorMap_" + suffix + ".png";
    const std::string depthFile = suffix.empty() ? "DepthMap.png" : "DepthMap_" + suffix + ".png";
    const std::string pointCloudPath =
        suffix.empty() ? "PointCloud.ply" : "PointCloud_" + suffix + ".ply";
    const std::string pointCloudColorPath =
        suffix.empty() ? "ColorPointCloud.ply" : "ColorPointCloud_" + suffix + ".ply";

    mmind::api::ColorMap color;
    showError(device.captureColorMap(color));
    saveMap(color, colorFile);

    mmind::api::DepthMap depth;
    showError(device.captureDepthMap(depth));
    saveMap(depth, depthFile);

    mmind::api::PointXYZMap pointXYZMap;
    showError(device.capturePointXYZMap(pointXYZMap));
    savePLY(pointXYZMap, pointCloudPath);

    mmind::api::PointXYZBGRMap pointXYZBGRMap;
    showError(device.capturePointXYZBGRMap(pointXYZBGRMap));
    savePLY(pointXYZBGRMap, pointCloudColorPath);
}
