/*******************************************************************************
 * BSD 3-Clause License
 *
 * Copyright (c) 2016-2025, Mech-Mind Robotics Technologies Co., Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Info:  https://www.mech-mind.com/
 *
 ******************************************************************************/

#pragma once
#include <string>
#include "CommonTypes.h"
#include "Version.h"

namespace mmind {

namespace eye {

/**
 * @brief Defines the camera information.
 */
struct CameraInfo
{
    std::string model;        ///< The device model, such as Mech-Eye NANO.
    std::string deviceName;   ///< The device name (UTF-8 encoded).
    std::string serialNumber; ///< The serial number of the device.
    Platform platform;        ///< The platform name of the device.
    Version hardwareVersion;  ///< The version of the hardware (pre-determined in the factory).
    Version firmwareVersion;  ///< The version of the firmware (upgradable).
    std::string ipAddress;    ///< The IP address of the device.
    std::string subnetMask{"255.255.255.0"}; ///< The subnet mask of the device.
    IpAssignmentMethod ipAssignmentMethod{}; ///< The IP address assignment method of the device.
    uint16_t port{};                         ///< The port used by the device.
};

/**
 * @brief Describes the device temperatures.
 */
struct DeviceTemperature
{
    float cpuTemperature{};       ///< The temperature (in °C) of the camera CPU.
    float projectorTemperature{}; ///< The temperature (in °C) of the camera projector.
};

/**
 * @brief Describes the camera's statuses.
 */
struct CameraStatus
{
    DeviceTemperature temperature;
};

/**
 * @brief Describes the camera intrinsic parameter matrix.
 */
struct CameraMatrix
{
    double fx{}; ///< Focal lengths.
    double fy{}; ///< Focal lengths.
    double cx{}; ///< Principal point.
    double cy{}; ///< Principal point.
};

/**
 * @brief Describes the distortion parameters.
 */
struct CameraDistortion
{
    double k1{}; ///< Radial distortion coefficients.
    double k2{}; ///< Radial distortion coefficients.
    double p1{}; ///< Tangential distortion coefficients.
    double p2{}; ///< Tangential distortion coefficients.
    double k3{}; ///< Radial distortion coefficients.
};

/**
 * @brief Describes the intrinsic parameters of the 2D camera in the 3D camera based on the pinhole
 * camera model, including the camera matrix and camera distortion parameters.
 */
struct Intrinsics2DCamera
{
    CameraDistortion cameraDistortion;
    CameraMatrix cameraMatrix;
};

/**
 * @brief Defines the rigid body transformations, including rotation matrix and translation vector.
 * @deprecated This struct has been deprecated and replaced by FrameTransformation, please use
 * @ref FrameTransformation instead.
 */
struct Transformation
{
    double rotation[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}; ///< 3*3 rotation matrix.
    double translation[3] = {0, 0, 0}; ///< 3*1 translation vector in [x(mm), y(mm), z(mm)].
};

/**
 * @brief Defines the camera image resolutions, including the resolutions of the 2D image (texture)
 * and depth map.
 */
struct CameraResolutions
{
    Size texture;
    Size depth;
};

/**
 * @brief Defines the 3D camera intrinsic parameters, including the intrinsic parameters of the
 * texture 2D camera, the intrinsic parameters of the depth 2D camera(s), and the transformation
 * between them.
 */
struct CameraIntrinsics
{
    Intrinsics2DCamera texture; ///< The intrinsic parameters of the texture 2D camera for capturing
                                ///< the 2D image (texture).
    Intrinsics2DCamera
        depth; ///< The intrinsic parameters of the depth 2D camera(s) for capturing the depth map.

    Transformation depthToTexture; ///< The rigid body transformation from the reference frame of
                                   ///< the depth 2D camera(s)
                                   ///<  to the reference frame of the texture 2D camera.
                                   ///< If the texture 2D camera is the same as the depth 2D camera,
                                   ///< no transformation is provided.
};

/**
 * @brief Defines the color type of the 2D camera in the 3D camera.
 */
enum class ColorTypeOf2DCamera { Monochrome, Color, Undefined };

} // namespace eye

} // namespace mmind
