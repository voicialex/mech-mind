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
#include <string>
#include <vector>

namespace mmind {

namespace api {

/**
 * @brief This enumeration defines the types of errors.
 */
struct ErrorStatus
{
    enum ErrorCode {
        MMIND_STATUS_SUCCESS = 0,         ///< Success status.
        MMIND_STATUS_INVALID_DEVICE = -1, ///< Error invalid device.
        MMIND_STATUS_DEVICE_OFFLINE =
            -2, ///< Error offline device. Please check the status of the device.
        MMIND_STATUS_FIRMWARE_NOT_SUPPORTED =
            -3, ///< Error not supported camera firmware. Please use MechEye Viewer to upgrade.
        MMIND_STATUS_PARAMETER_SET_ERROR = -4,          ///< Error setting parameter to device.
        MMIND_STATUS_PARAMETER_GET_ERROR = -5,          ///< Error reading parameter from device.
        MMIND_STATUS_CAPTURE_NO_FRAME = -6,             ///< Error no frame is captured.
        MMIND_STATUS_INVALID_INPUT_FRAME = -7,          ///< Error invalid input frame.
        MMIND_STATUS_INVALID_INTRINSICS_PARAMETER = -8, ///< Error invalid intrinsics parameters.
        MMIND_HANDEYE_CALIBRATION_POSE_INVALID = -9,    ///< Error invalid pose..
        MMIND_HANDEYE_CALIBRATION_PATTERN_IMAGE_ERROR =
            -10, ///< Error occurred while processing the 2D image with feature detection results.
        MMIND_HANDEYE_CALIBRATION_POSES_INSUFFICIENT =
            -11, ///< Error insufficient calibration poses.
    };
    ErrorStatus() = default;
    ErrorStatus(ErrorCode code, const std::string& message)
        : errorCode(code), errorDescription(message)
    {
    }

    bool isOK() const { return errorCode == MMIND_STATUS_SUCCESS; }

    /**
     * Error code.
     */
    ErrorCode errorCode{MMIND_STATUS_SUCCESS};

    /**
     * Detailed error message.
     */
    std::string errorDescription;
};

/**
 * @brief This struct defines device information.
 */
struct MechEyeDeviceInfo
{
    std::string model; ///< Device model name, such as Mech-Eye Nano.
    std::string id;    ///< Device ID.
    std::string
        hardwareVersion; ///< The version of the hardware which is pre-determined from the factory.
    std::string firmwareVersion; ///< The version of the firmware which can be upgraded.
    std::string ipAddress;       ///< IP address of the device.
    uint16_t port;               ///< Device port.
};

/**
 * @brief This struct describes the device temperature information.
 */
struct DeviceTemperature
{
    float cpuTemperature; ///< CPU temperature in the device motherboard in degrees Celsius.
    float projectorModuleTemperature; ///< projector module temperature in degrees Celsius.
};

/**
 * @brief This struct defines camera intrinsic parameters.
 */
struct CameraIntri
{
    double distortion[5] = {}; ///< Vector of distortion coefficients, which arrange in [k1, k2, p1,
                               ///< p2, k3].
    double cameraMatrix[4] = {}; ///< Camera matrix, which arrange in [fx, fy, cx, cy].
};

/**
 * @brief This struct defines rigid body transformations, including rotation matrix and
 * translation vector.
 */
struct Pose
{
    double rotation[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}; ///< 3*3 rotation matrix.
    double translation[3] = {0, 0, 0}; ///< 3*1 translation vector in [x(mm), y(mm), z(mm)].
};

/**
 * @brief This struct defines device intrinsic parameters, including texture camera and depth
 * camera.
 */
struct DeviceIntri
{
    CameraIntri
        textureCameraIntri; /// The intrinsic parameters of the camera for capturing color map.
    CameraIntri
        depthCameraIntri; /// The intrinsic parameters of the camera for capturing depth map.

    Pose depthToTexture; /// The rigid body transformation from depth camera coordinate
                         /// system to texture camera coordinate system. There is no transform
                         /// when texture camera and depth camera are the same.
};

/**
 * @brief This struct defines camera map resolution.
 */
struct DeviceResolution
{
    unsigned colorMapWidth;  ///< The width of the color map.
    unsigned colorMapHeight; ///< The height of the color map.
    unsigned depthMapWidth;  ///< The width of the depth map.
    unsigned depthMapHeight; ///< The height of the depth map.
};

/**
 * @brief This struct defines camera region of interest.
 */
struct ROI
{
    ROI() = default;
    ROI(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}
    unsigned x;      ///< The column coordinates of the upper left point of the region of interest.
    unsigned y;      ///< The row coordinates of the upper left point of the region of interest.
    unsigned width;  ///< The width of the region of interest.
    unsigned height; ///< The height of the region of interest.
};

/**
 * @brief This struct defines the depth range.
 */
struct DepthRange
{
    DepthRange() = default;
    DepthRange(int lower, int upper) : lower(lower), upper(upper) {}
    int lower; ///< The lower limit of the roi on the z value of the depth map in the
               ///< camera coordinate system.
    int upper; ///< The upper limit of the roi on the z value of the depth map in the
               ///< camera coordinate system
};

/**
 * @brief This enumeration defines the exposure mode.
 */
enum class ExposureMode {
    HDR,   ///< Generate images of high dynamic ranges showing more details with multiple
           ///< different exposures.
    Timed, ///< Fixed exposure time, recommended in stable ambient light conditions.
};

/**
 * @brief This enumeration defines the scan mode.
 */
enum class CaptureMode {
    Raw,   ///< The Raw mode.
    Depth, ///< The Depth mode.
};

/**
 * @brief This enumeration defines the trigger source.
 */
enum class TriggerSource {
    Software, ///< The software trigger source.
    Encoder,  ///< The Encoder trigger source.
};

/**
 * @brief This enumeration defines the image analog gain.
 */
enum class ImageAnalogGain {
    AnalogGain_1_0, ///< the analog gain value is 1.0
    AnalogGain_1_3, ///< the analog gain value is 1.3
    AnalogGain_1_9, ///< the analog gain value is 1.9
    AnalogGain_2_8, ///< the analog gain value is 2.8
    AnalogGain_5_5, ///< the analog gain value is 5.5
};

enum class ImageAnalogGainFor8030 {
    AnalogGain_1_0, ///< the analog gain value is 1.0
    AnalogGain_1_3, ///< the analog gain value is 1.3
    AnalogGain_2_0, ///< the analog gain value is 2.0
    AnalogGain_3_0, ///< the analog gain value is 3.0
};

/**
 * @brief This enumeration defines the ROI along the Z direction.
 */
enum class ROIMode {
    Mode_1_1,  ///< The ROI covers the entire image.
    Mode_1_2,  ///< The ROI height is one-half of the image height.
    Mode_1_4,  ///< The ROI height is one-fourth of the image height.
    Mode_1_8,  ///< The ROI height is one-eighth of the image height.
    Mode_1_16, ///< The ROI height is one-sixteenth of the image height.
};

/**
 * @brief This enumeration defines the encoder trigger mode.
 */
enum class EncoderTriggerMode {
    Positive, ///< Camera is triggered when the encoder rotates in the positive direction. Pulse
              ///< count increases.
    Negative, ///< Camera is triggered when the encoder rotates in the negative direction. Pulse
              ///< count decreases.
    Duplex,   ///< Camera is triggered in both directions. When the encoder rotates in the positive
              ///< direction, pulse count increases; when the encoder rotates in the negative
              ///< direction, pulse count decreases.
};

/**
 * @brief This enumeration defines the encoder multiplier.
 */
enum class EncoderMultiplier {
    Multiple_1, ///< Camera is triggered at the rising edge of the leading channel.
    Multiple_2, ///< Camera is triggered by the rising edge of both channels.
    Multiple_4, ///< Camera is triggered by the rising edge and falling edge of both channels.
};

/**
 * @brief This enumeration defines the filter type.
 */
enum class FilterType {
    None,                      ///< No filtering algorithm applied.
    Smoothing,                 ///< Apply the algorithm of mean filtering without edge preserving.
    Smoothing_edge_preserving, ///< Apply the algorithm of mean filtering with edge preserving.
    Median,                    ///< Apply the algorithm of median filtering.
};

/**
 * @brief This enumeration defines the window size for mean filtering.
 */
enum class MeanFilterWindow {
    Window_Size_0,  ///< Do not apply mean filtering.
    Window_Size_2,  ///< The window size for mean filtering is 2.
    Window_Size_4,  ///< The window size for mean filtering is 4.
    Window_Size_8,  ///< The window size for mean filtering is 8.
    Window_Size_16, ///< The window size for mean filtering is 16.
    Window_Size_32, ///< The window size for mean filtering is 32.
    Window_Size_64, ///< The window size for mean filtering is 64.
};

/**
 * @brief This enumeration defines the window size for median filtering.
 */
enum class MedianFilterWindow {
    Window_Size_0, ///< Do not apply median filtering.
    Window_Size_3, ///< The window size for median filtering is 3.
    Window_Size_5, ///< The window size for median filtering is 5.
    Window_Size_7, ///< The window size for median filtering is 7.
    Window_Size_9, ///< The window size for median filtering is 9.
};

/**
 * @brief This enumeration defines the profile extraction strategy.
 */
enum class ProfileStrategy {
    Standard,  ///< Select the brightest spot in each column as the profile.
    Near,      ///< Select the spot closest to the camera in each column as the profile.
    Far,       ///< Select the spot farthest to the camera in each column as the profile.
    Continous, ///< Not implemented.
    Invalid,   ///< Discard the data of the column when multiple spots are present in the column.
};

} // namespace api
} // namespace mmind
