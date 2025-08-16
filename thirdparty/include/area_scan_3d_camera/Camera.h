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
#include <memory>
#include <vector>
#include "api_global.h"
#include "CameraProperties.h"
#include "ErrorStatus.h"
#include "Frame2DAnd3D.h"
#include "UserSetManager.h"

namespace mmind {

namespace eye {

/**
 * @brief Operates the camera.
 * Use @ref Camera.connect to connect an available camera, and then call the corresponding methods
 * to perform data acquisition, configure parameters, and so on.
 */
class MMIND_API_EXPORT Camera
{
public:
    /**
     * @brief Constructor.
     */
    Camera();

    /**
     * @brief Destructor.
     */
    ~Camera();

    /**
     * @brief Copy constructor.
     */
    Camera(const Camera& other) noexcept;

    /**
     * @brief Copy assignment.
     */
    Camera& operator=(const Camera& other) noexcept;

    /**
     * @brief Discovers all available cameras and returns the list of
     * information of all available cameras. If a camera is not successfully discovered, please
     * check the network connection and indicator lights on the camera.
     * @param [in] timeoutMs The timeout period (in ms) for discovering cameras. If no cameras are
     * discovered after the timeout period has passed, this method returns an empty list.
     * @return The list of information of all available cameras.
     */
    static std::vector<CameraInfo> discoverCameras(unsigned int timeoutMs = 5000);

    /**
     * @brief Connects to a camera using @ref CameraInfo.
     * @param [in] info The information of the camera to be connected. You can use @ref
     * Camera.discoverCameras to discover all available cameras.
     * @param [in] timeoutMs The timeout period (in ms) for connecting to a camera. If the camera
     * connection does not succeed after the timeout period has passed, this method returns @ref
     * ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_INPUT_ERROR IP address format error.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE IP address not corresponding to an available
     *  device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported camera model or firmware
     *  version.\n
     *  @ref ErrorStatus::MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus connect(const CameraInfo& info, unsigned int timeoutMs = 5000);

    /**
     * @brief Connects to a camera using the IP address.
     * @param [in] ipAddress The IP address of the camera to be connected. The format of a valid IP
     * address: 100.100.1.1.
     * @param [in] timeoutMs The timeout period (in ms) for connecting to a camera. If the camera
     * connection does not succeed after the timeout period has passed, this method returns @ref
     * ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_INPUT_ERROR IP address format error.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE IP address not corresponding to an available
     *  device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported camera model or firmware
     *  version.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus connect(const std::string& ipAddress, unsigned int timeoutMs = 5000);

    /**
     * @brief Disconnects from the camera and releases the associated resources.
     */
    void disconnect();

    /**
     * @brief Sets the time interval at which the client sends periodic heartbeat messages to the
     * camera side. The default time interval is 10s.
     * @param [in] timeIntervalMs The time interval for periodic sending heartbeat messages in
     * milliseconds. The valid setting range is from 1s to 3600s.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_OUT_OF_RANGE_ERROR Invalid parameter input.\n
     */
    ErrorStatus setHeartbeatInterval(unsigned int timeIntervalMs);

    /**
     * @brief Gets the basic information of the camera, such as the model, serial number,
     * firmware version, and IP configuration.
     * @param [out] See @ref CameraInfo for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     */
    ErrorStatus getCameraInfo(CameraInfo& info) const;

    /**
     * @brief Gets various statuses of the camera.
     * @param [out] See @ref CameraStatus for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     */
    ErrorStatus getCameraStatus(CameraStatus& status) const;

    /**
     * @brief Gets the camera intrinsic parameters. The camera intrinsic parameters include the
     * intrinsic parameters of the texture 2D camera and the intrinsic parameters of the depth 2D
     * camera(s) based on the pinhole camera model, which correspond to the 2D and 3D capture
     * results @ref Frame2D and @ref Frame3D. The camera intrinsic parameters also include the
     * transformation relationship between the texture 2D camera and depth 2D camera(s).
     * @param [out] See @ref CameraIntrinsics for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     */
    ErrorStatus getCameraIntrinsics(CameraIntrinsics& intrinsics) const;

    /**
     * @brief Gets the image resolutions of the camera. Two image resolutions are provided,
     * 2D image (texture) and depth map, which correspond to the 2D and 3D capture results @ref
     * Frame2D and @ref Frame3D.
     * @param [out] See @ref CameraResolutions for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     */
    ErrorStatus getCameraResolutions(CameraResolutions& resolutions) const;

    /**
     * @brief Gets the @ref UserSet currently in use.Through @ref UserSet, you can access all
     * available parameters of the camera related to 2D and 3D data acquisition. You can also use
     * @ref UserSet to directly set and get the parameter values instead of using @ref Parameter
     * interface.
     * @return See @ref UserSet for details.
     */
    UserSet& currentUserSet();

    /**
     * @brief Gets the @ref UserSetManager of the camera. @ref UserSetManager provides various
     * operations for managing all user sets saved in the camera, including adding and deleting user
     * sets and selecting the user set to be used. It also provides the methods for saving all user
     * sets to a JSON file and load all user sets from a JSON file.
     * @return See @ref UserSetManager for details.
     */
    UserSetManager& userSetManager();

    /**
     * @brief Projects structured light and captures a single 3D frame. 3D
     * information is computed on the camera. The result is retrieved after the
     * computation is completed.
     * @param [out] frame3D The capture result. See @ref Frame3D for details.
     * @param [in] timeoutMs The timeout period (in ms) for the capturing process. If no data is
     * received within the timeout period, this method returns
     * @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR No frame data obtained. Some error may have
     *  occurred on the device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported camera model.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *
     */
    ErrorStatus capture3D(Frame3D& frame3D, unsigned int timeoutMs = 5000) const;

    /**
     * @brief Projects structured light and captures a single 3D frame. 3D
     * information and normal vector are computed on the camera. The result is
     * retrieved after the computation is completed.
     * @param [out] frame3D The capture result. See @ref Frame3D for details.
     * @param [in] timeoutMs The timeout period (in ms) for the capturing process. If no data is
     * received within the timeout period, this method returns
     * @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR No frame data obtained. Some error may have
     *  occurred on the device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported camera model.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus capture3DWithNormal(Frame3D& frame3D, unsigned int timeoutMs = 10000) const;

    /**
     * @brief Captures a single 2D frame using the 2D camera in the 3D camera. The result is
     * retrieved after the capture is completed.
     * @param [out] frame2D The capture result. See @ref Frame2D for details.
     * @param [in] timeoutMs The timeout period (in ms) for the capturing process. If no data is
     * received within the timeout period, this method returns
     * @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR No frame data obtained. Some error may have
     *  occurred on the device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported camera model.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus capture2D(Frame2D& frame2D, unsigned int timeoutMs = 5000) const;

    /**
     * @brief Simultaneously captures a single 2D frame and 3D frame. 3D
     * information is computed on the camera. Both 2D and 3D capture results are
     * retrieved after the capturing and computation processes are completed. Using this method, you
     * can easily generate the textured point cloud from both 2D and 3D data.
     * @param [out] frame2DAnd3D The capture result containing @ref Frame2D and @ref Frame3D. See
     * @ref Frame2DAnd3D for details.
     * @param [in] timeoutMs The timeout period (in ms) for the capturing process. If no data is
     * received within the timeout period, this method returns
     * @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR No frame data obtained. Some error may have
     *  occurred on the device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported camera model. \n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus capture2DAnd3D(Frame2DAnd3D& frame2DAnd3D, unsigned int timeoutMs = 10000) const;

    /**
     * @brief Simultaneously captures a single 2D frame and 3D frame. 3D
     * information and normal vector are computed on the camera.
     * Both 2D and 3D capture results are retrieved after the capturing and computation processes
     * are completed. Using this method, you can easily generate the texture point cloud from both
     * 2D and 3D data.
     * @param [out] frame2DAnd3D The capture result containing @ref Frame2D and @ref Frame3D. See
     * @ref Frame2DAnd3D for details.
     * @param [in] timeoutMs The timeout period (in ms) for the capturing process. If no data is
     * received within the timeout period, this method returns
     * @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR No frame data obtained. Some error may have
     *  occurred on the device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported camera model. \n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus capture2DAnd3DWithNormal(Frame2DAnd3D& frame2DAnd3D,
                                         unsigned int timeoutMs = 15000) const;

    /**
     * @brief Captures the 2D images from both 2D cameras in the 3D camera. This method is
     * only available for DEEP, DEEP-GL, LSR S, LSR S-GL, LSR L, LSR L-GL, PRO XS, and PRO XS-GL.
     * @since V2.2.1
     * @param [out] left The 2D image of the left 2D camera. See @ref Frame2D for details.
     * @param [out] right The 2D image of the right 2D camera. See @ref Frame2D for details.
     * @param [in] isRectified Whether to perform stereo rectification. After the rectification, The
     * left image and the depth map are aligned pixel-to-pixel. If true, the rectified 2D images are
     * returned. If false, the original images are returned.
     * @param [in] timeoutMs The timeout period (in ms) for the capturing process. If no data is
     * received within the timeout period, this method returns @ref
     * ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR No frame data obtained. Some error may have
     *  occurred on the device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported camera model or firmware
     *  version.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     * @note Different camera models have different parameters for adjusting the exposure of
     * these 2D images. For DEEP, DEEP-GL, LSR S, LSR S-GL, LSR L, and LSR L-GL,adjust @ref
     * DepthSourceExposureMode and @ref DepthSourceExposureTime. For the other models,
     * adjust @ref ExposureMode and @ref ExposureTime.
     */
    ErrorStatus captureStereo2D(Frame2D& left, Frame2D& right, bool isRectified = false,
                                unsigned int timeoutMs = 5000) const;

    /**
     * @brief Sets the unit of the point cloud.
     * @param [in] unit The new @ref CoordinateUnit of the point cloud.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR No frame data obtained. Some error may have
     *  occurred on the device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported camera model.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus setPointCloudUnit(CoordinateUnit unit);

    /**
     * @brief Gets the current unit of the point cloud.
     * @param [out] unit The current @ref CoordinateUnit of the point cloud.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR No frame data obtained. Some error may have
     *  occurred on the device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Unsupported camera model.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus getPointCloudUnit(CoordinateUnit& unit) const;

    /**
     * @brief Saves the acquired data, @ref Parameter s, and @ref CameraInfo in an MRAW format file
     * that can be loaded as a virtual device in Mech-Eye Viewer.
     *
     * @param fileName The name of the MRAW file to be saved. You can add a path before the name to
     * specify the path for saving the file.
     * @note Depending on the parameter settings and amount of data to be saved, it may take up to a
     * few minutes to save the virtual device file. Please ensure that the file name is encoded in
     * UTF-8 format.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid camera handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Camera disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Parse response error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR There are errors in reply.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Failed to create the virtual device file.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR No data available for saving.
     */
    ErrorStatus saveVirtualDeviceFile(const std::string& fileName);

private:
    friend class HandEyeCalibration;
    friend class CameraEvent;
    friend class InternalInterfaces;
    std::shared_ptr<class CameraImpl> _cameraImpl;
};

} // namespace eye

} // namespace mmind
