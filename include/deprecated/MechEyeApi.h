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
#include <memory>
#include <vector>
#include "api_global.h"
#include "MechEyeFrame.hpp"
#include "MechEyeDataType.h"
#include "MechEyeSettings.h"

namespace mmind {

namespace api {

class MechEyeDeviceImpl;

/**
 * @brief Gets the API basic information including version and manufacturing company.
 * @return API information string.
 */
std::string MMIND_API_EXPORT getApiInformation();

/**
 * @brief Interface that is used to connect the Mech-Eye Industrial 3D Camera and access basic
 * information of the device.
 */
class MMIND_API_EXPORT MechEyeDevice
{
public:
    /**
     * Constructor.
     */
    MechEyeDevice();

    /**
     * Destructor.
     */
    ~MechEyeDevice();

    /**
     * @brief Enumerates Mech-Eye Industrial 3D Camera by the type of @ref MechEyeDeviceInfo
     * identifying the device.
     * @return
     *  Information on all detectable Mech-Eye Industrial 3D Camera.
     */
    static std::vector<MechEyeDeviceInfo> enumerateMechEyeDeviceList();

    /**
     * @brief Gets the point cloud image within the mask specified by the texture map.
     * @param [in] depthMap See @ref Frame for details.
     * @param [in] textureMask See @ref Frame for details.
     * @param [in] intri See @ref DeviceIntri for details.
     * @param [out] pointXYZMask See @ref Frame for details.
     * @return See @ref ErrorStatus for details.
     */
    static ErrorStatus getCloudFromTextureMask(const DepthMap& depthMap,
                                               const ColorMap& textureMask,
                                               const DeviceIntri& intri, PointXYZMap& pointXYZMask);

    /**
     * @brief Gets the colored point cloud image within the mask specified by the texture map.
     * @param [in] depthMap See @ref Frame for details.
     * @param [in] textureMask See @ref Frame for details.
     * @param [in] texture See @ref Frame for details.
     * @param [in] intri See @ref DeviceIntri for details.
     * @param [out] pointXYZBGRMask See @ref Frame for details.
     * @return See @ref ErrorStatus for details.
     */
    static ErrorStatus getCloudFromTextureMask(const DepthMap& depthMap,
                                               const ColorMap& textureMask, const ColorMap& texture,
                                               const DeviceIntri& intri,
                                               PointXYZBGRMap& pointXYZBGRMask);

    /**
     * @brief Connects to the device by the @ref MechEyeDeviceInfo identifying a device.
     * @param [in] info the device information used to connect the device which can be obtained by
     * @ref enumerateMechEyeDeviceList function.
     * @param [in] timeout the timeout value (ms)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus connect(const MechEyeDeviceInfo& info, int timeout = 10000);

    /**
     * @brief Connects to the device by the ip address and port identifying a device.
     * @param [in] ipAddress the device ip address used to connect the device
     * @param [in] port the device port used to connect the device
     * @param [in] timeout the timeout value (ms)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus connect(const std::string& ipAddress, int port = 5577, int timeout = 10000);

    /**
     * @brief Disconnects from the device.
     */
    void disconnect();

    /**
     * @brief Gets the temperature information about the connected device.
     * @param [out] temperature See @ref DeviceTemperature for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getDeviceTemperature(DeviceTemperature& temperature) const;

    /**
     * @brief Gets the basic information about the connected device.
     * @param [out] info See @ref MechEyeDeviceInfo for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getDeviceInfo(MechEyeDeviceInfo& info) const;

    /**
     * @brief Gets the intrinsic camera parameter about the connected device.
     * @param [out] intri See @ref DeviceIntri for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getDeviceIntri(DeviceIntri& intri) const;

    /**
     * @brief Gets the device resolution.
     * @param [out] imageResolution See @ref DeviceResolution for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getDeviceResolution(DeviceResolution& imageResolution) const;

    /**
     * @brief Captures a color image.
     * @param [out] colorMap See @ref Frame for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus captureColorMap(ColorMap& colorMap) const;
    /**
     * @brief Captures a depth image.
     * @param [out] depthMap See @ref Frame for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus captureDepthMap(DepthMap& depthMap) const;

    /**
     * @brief Captures a point cloud image.
     * @param [out] pointXYZMap See @ref Frame for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus capturePointXYZMap(PointXYZMap& pointXYZMap) const;

    /**
     * @brief Captures an orthogonal depth image.
     * @param [out] depthMap See @ref Frame for details.
     * @param [out] xScale Scale value in x axis.
     * @param [out] yScale Scale value in y axis.
     * @param [out] xOffset Offset value in x axis.
     * @param [out] yOffset Offset value in y axis.

     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus captureOrthogonalDepthMap(DepthMap& depthMap, double& xScale, double& yScale,
                                          double& xOffset, double& yOffset) const;

    /**
     * @brief Captures a colored point cloud image.
     * @param [out] pointXYZBGRMap See @ref Frame for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus capturePointXYZBGRMap(PointXYZBGRMap& pointXYZBGRMap) const;

    /**
     * @brief Sets the camera exposure mode to capture the 2D images.
     * @param [in] value See \ref Scanning2DSettings.exposureMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan2DExposureMode(Scanning2DSettings::Scan2DExposureMode value) const;

    /**
     * @brief Gets the camera exposure mode to capture the 2D images.
     * @param [out] value See \ref Scanning2DSettings.exposureMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan2DExposureMode(Scanning2DSettings::Scan2DExposureMode& value) const;

    /**
     * @brief Sets the camera exposure time in \ref Scanning2DSettings.Timed exposure mode.
     * @param [in] value See \ref Scanning2DSettings.exposureTime for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan2DExposureTime(double value) const;

    /**
     * @brief  Gets the camera exposure time in \ref Scanning2DSettings.Timed exposure mode.
     * @param [out] value See \ref Scanning2DSettings.exposureTime for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan2DExposureTime(double& value) const;

    /**
     * @brief Sets the camera HDR exposure sequence in \ref Scanning2DSettings.HDR exposure mode.
     * @param [in] values See \ref Scanning2DSettings.hdrExposureSequence for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan2DHDRExposureSequence(std::vector<double>& values) const;

    /**
     * @brief  Gets the camera HDR exposure sequence in \ref Scanning2DSettings.HDR exposure mode.
     * @param [out] values See \ref Scanning2DSettings.hdrExposureSequence for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan2DHDRExposureSequence(std::vector<double>& values) const;

    /**
     * @brief Sets the expected gray value in \ref Scanning2DSettings.Auto exposure mode.
     * @param [in] value See \ref Scanning2DSettings.expectedGrayValue for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan2DExpectedGrayValue(int value) const;

    /**
     * @brief Gets the expected gray value in \ref Scanning2DSettings.Auto exposure mode.
     * @param [out] valus See \ref Scanning2DSettings.expectedGrayValue for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan2DExpectedGrayValue(int& value) const;

    /**
     * @brief Sets whether gray level transformation algorithm is used or not.
     * @param [in] value See \ref Scanning2DSettings.toneMappingEnable for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan2DToneMappingEnable(bool value) const;

    /**
     * @brief  Gets whether gray level transformation algorithm is used or not.
     * @param [out] value See \ref Scanning2DSettings.toneMappingEnable for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan2DToneMappingEnable(bool& value) const;

    /**
     * @brief Sets the image sharpen factor.
     * @param [in] value See \ref Scanning2DSettings.sharpenFactor for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan2DSharpenFactor(double value) const;

    /**
     * @brief  Gets the image sharpen factor.
     * @param [out] value See \ref Scanning2DSettings.sharpenFactor for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan2DSharpenFactor(double& value) const;

    /**
     * @brief Sets ROI to capture the 2D image.
     * @param [in] value See @ref Scanning2DSettings.scan2DROI for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan2DROI(const ROI& value) const;

    /**
     * @brief Gets ROI to capture the 2D image.
     * @param [out] value See @ref Scanning2DSettings.scan2DROI for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan2DROI(ROI& value) const;

    /**
     * @brief Sets the exposure time of the camera to capture the 3D image.
     * @param [in] valueSequence See @ref Scanning3DSettings.exposureSequence for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan3DExposure(const std::vector<double>& valueSequence) const;

    /**
     * @brief Gets the exposure time sequence of the camera to capture the 3D image.
     * @param [out] valueSequence See @ref Scanning3DSettings.exposureSequence for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan3DExposure(std::vector<double>& valueSequence) const;

    /**
     * @brief Sets gain to capture the 3d image.
     * @param [in] value  See @ref Scanning3DSettings.gain for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan3DGain(double value) const;

    /**
     * @brief Gets gain to capture the 3d image.
     * @param [out] value See @ref Scanning3DSettings.gain for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan3DGain(double& value) const;

    /**
     * @brief Sets ROI to capture the 3D image.
     * @param [in] value See @ref Scanning3DSettings.scan3DROI for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan3DROI(const ROI& value) const;

    /**
     * @brief Gets ROI to capture the 3D image. MechEye UHP serials in capture mode 'Merge' does not
     * support this parameter.
     * @param [out] value See @ref Scanning3DSettings.scan3DROI for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan3DROI(ROI& value) const;

    /**
     * @brief Sets depth range in 3D image.
     * @param [in] value See @ref Scanning3DSettings.depthRange for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setDepthRange(const DepthRange& value) const;

    /**
     * @brief Gets depth range in 3D image.
     * @param [out] value See @ref Scanning3DSettings.depthRange for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getDepthRange(DepthRange& value) const;

    /**
     * @brief Sets the signal contrast threshold for effective pixels.
     * @param [in] value See @ref PointCloudProcessingSettings.fringeContrastThreshold for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setFringeContrastThreshold(int value) const;

    /**
     * @brief Gets the signal contrast threshold for effective pixels.
     * @param [out] value See @ref PointCloudProcessingSettings.fringeContrastThreshold for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getFringeContrastThreshold(int& value) const;

    /**
     * @brief Sets the signal minimum threshold for effective pixels.
     * @param [in] value See @ref PointCloudProcessingSettings.fringeMinThreshold for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setFringeMinThreshold(int value) const;

    /**
     * @brief Gets the signal minimum threshold for effective pixels.
     * @param [out] value See @ref PointCloudProcessingSettings.fringeMinThreshold for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getFringeMinThreshold(int& value) const;

    /**
     * @brief Sets the point cloud outliers removal algorithm.
     * @param [in] value See @ref PointCloudProcessingSettings.outlierFilterMode for details.
     * @return See @ref ErrorStatus for details.
     * @deprecated Please use @ref MechEyeDevice.getCloudOutlierRemovalMode() instead.
     */
    [[deprecated("please use setCloudOutlierRemovalMode() instead.")]] ErrorStatus
    setCloudOutlierFilterMode(PointCloudProcessingSettings::CloudOutlierFilterMode value) const;

    /**
     * @brief Gets the point cloud outliers removal algorithm.
     * @param [out] value See @ref PointCloudProcessingSettings.outlierFilterMode for details.
     * @return See @ref ErrorStatus for details.
     * @deprecated Please use @ref MechEyeDevice.getCloudOutlierRemovalMode() instead.
     */
    [[deprecated("please use getCloudOutlierRemovalMode() instead.")]] ErrorStatus
    getCloudOutlierFilterMode(PointCloudProcessingSettings::CloudOutlierFilterMode& value) const;

    /**
     * @brief Sets the point cloud smoothing algorithm.
     * @param [in] value See @ref PointCloudProcessingSettings.smoothMode for details.
     * @return See @ref ErrorStatus for details.
     * @deprecated Please use @ref MechEyeDevice.setCloudSurfaceSmoothingMode() instead.
     */
    [[deprecated("please use setCloudSurfaceSmoothingMode() instead.")]] ErrorStatus
    setCloudSmoothMode(PointCloudProcessingSettings::CloudSmoothMode value) const;

    /**
     * @brief Gets the point cloud smoothing algorithm.
     * @param [out] value See @ref PointCloudProcessingSettings.smoothMode for details.
     * @return See @ref ErrorStatus for details.
     * @deprecated Please use @ref MechEyeDevice.getCloudSurfaceSmoothingMode() instead.
     */
    [[deprecated("please use getCloudSurfaceSmoothingMode() instead.")]] ErrorStatus
    getCloudSmoothMode(PointCloudProcessingSettings::CloudSmoothMode& value) const;

    /**
     * @brief Sets the point cloud surface smoothing mode.
     * @param [in] value See @ref PointCloudProcessingSettings.surfaceSmoothing for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setCloudSurfaceSmoothingMode(
        PointCloudProcessingSettings::PointCloudSurfaceSmoothing value) const;

    /**
     * @brief Gets the point cloud surface smoothing mode.
     * @param [out] value See @ref PointCloudProcessingSettings.surfaceSmoothing for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getCloudSurfaceSmoothingMode(
        PointCloudProcessingSettings::PointCloudSurfaceSmoothing& value) const;

    /**
     * @brief Sets the point cloud noise removal mode.
     * @param [in] value See @ref PointCloudProcessingSettings.noiseRemoval for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setCloudNoiseRemovalMode(
        PointCloudProcessingSettings::PointCloudNoiseRemoval value) const;

    /**
     * @brief Gets the point cloud noise removal mode.
     * @param [out] value See @ref PointCloudProcessingSettings.noiseRemoval for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getCloudNoiseRemovalMode(
        PointCloudProcessingSettings::PointCloudNoiseRemoval& value) const;

    /**
     * @brief Sets the point cloud outlier removal mode.
     * @param [in] value See @ref PointCloudProcessingSettings.outlierRemoval for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setCloudOutlierRemovalMode(
        PointCloudProcessingSettings::PointCloudOutlierRemoval value) const;

    /**
     * @brief Gets the point cloud outlier removal mode.
     * @param [out] value See @ref PointCloudProcessingSettings.outlierRemoval for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getCloudOutlierRemovalMode(
        PointCloudProcessingSettings::PointCloudOutlierRemoval& value) const;

    /**
     * @brief Sets the point cloud edge preservation mode.
     * @param [in] value See @ref PointCloudProcessingSettings.edgePreservation for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setCloudEdgePreservationMode(
        PointCloudProcessingSettings::PointCloudEdgePreservation value) const;

    /**
     * @brief Gets the point cloud edge preservation mode.
     * @param [out] value See @ref PointCloudProcessingSettings.edgePreservation for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getCloudEdgePreservationMode(
        PointCloudProcessingSettings::PointCloudEdgePreservation& value) const;

    /**
     * @brief Sets projector's fringe coding mode. Only support with Mech-Eye NANO and PRO
     * series industrial 3D cameras.
     * @param [in] value See @ref ProjectorSettings.fringeCodingMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setProjectorFringeCodingMode(ProjectorSettings::FringeCodingMode value) const;

    /**
     * @brief Gets projector's fringe coding mode. Only support with Mech-Eye NANO and PRO
     * series industrial 3D cameras.
     * @param [out] value See @ref ProjectorSettings.fringeCodingMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getProjectorFringeCodingMode(ProjectorSettings::FringeCodingMode& value) const;

    /**
     * @brief Sets projector's powerl level. Only support with Mech-Eye NANO, LOG and PRO
     * series industrial 3D cameras.
     * @param [in] value See @ref ProjectorSettings.powerLevel for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setProjectorPowerLevel(ProjectorSettings::PowerLevel value) const;

    /**
     * @brief Gets projector's powerl level. Only support with Mech-Eye NANO, LOG and PRO
     * series industrial 3D cameras.
     * @param [out] value See @ref ProjectorSettings.powerLevel for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getProjectorPowerLevel(ProjectorSettings::PowerLevel& value) const;

    /**
     * @brief Sets projector's anti-flicker mode. Only support with Mech-Eye NANO and PRO
     * series industrial 3D camera.
     * @param [in] value See @ref ProjectorSettings.antiFlickerMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setProjectorAntiFlickerMode(ProjectorSettings::AntiFlickerMode value) const;

    /**
     * @brief Gets projector's anti-flicker mode. Only support with Mech-Eye NANO and
     * PRO series industrial 3D camera.
     * @param [out] value See @ref ProjectorSettings.antiFlickerMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getProjectorAntiFlickerMode(ProjectorSettings::AntiFlickerMode& value) const;

    /**
     * @brief Sets laser device settings. Only support with Mech-Eye Laser series industrial 3D
     * camera.
     * @param [in] value See @ref LaserSettings for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLaserSettings(const LaserSettings& value) const;

    /**
     * @brief Gets laser device settings. Only support with Mech-Eye Laser series industrial 3D
     * camera.
     * @param [out] value See @ref LaserSettings for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLaserSettings(LaserSettings& value) const;

    /**
     * @brief Sets UHP device settings. Only support with Mech-Eye UHP series industrial 3D
     * camera.
     * @param [in] value See @ref UhpSettings for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setUhpSettings(UhpSettings value) const;

    /**
     * @brief Gets UHP device settings. Only support with Mech-Eye UHP series industrial 3D
     * camera.
     * @param [out] value See @ref UhpSettings for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getUhpSettings(UhpSettings& value) const;

    /**
     * @brief Sets UHP camera capture mode. Only support with Mech-Eye UHP series industrial 3D
     * camera.
     * @param [in] value See @ref UhpSettings.captureMode and @ref UhpSettings.UhpCaptureMode for
     * details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setUhpCaptureMode(UhpSettings::UhpCaptureMode value) const;

    /**
     * @brief Gets UHP camera capture mode. Only support with Mech-Eye UHP series industrial 3D
     * camera.
     * @param [out] value See @ref UhpSettings.captureMode and @ref UhpSettings.UhpCaptureMode for
     * details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getUhpCaptureMode(UhpSettings::UhpCaptureMode& value) const;

    /**
     * @brief Sets UHP camera fringe coding mode. Only support with Mech-Eye UHP series industrial
     * 3D camera.
     * @param [in] value See @ref UhpSettings.fringeCodingMode and @ref
     * UhpSettings.UhpFringeCodingMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setUhpFringeCodingMode(UhpSettings::UhpFringeCodingMode value) const;

    /**
     * @brief Gets UHP camera fringe coding mode. Only support with Mech-Eye UHP series industrial
     * 3D camera.
     * @param [out] value See @ref UhpSettings.fringeCodingMode and @ref
     * UhpSettings.UhpFringeCodingMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getUhpFringeCodingMode(UhpSettings::UhpFringeCodingMode& value) const;

    /**
     * @brief Saves all parameters to the current user set.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus saveAllSettingsToUserSets() const;

    /**
     * @brief Set the board type for this calibration.
     * @param [in] value See @ref boardType.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setBoardType(HandeyeCalibrationSettings::boardType boardType) const;

    /**
     * @brief Set the calibration type for this calibration.
     * @param [in] value See @ref calibrationType.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setCalibrateType(HandeyeCalibrationSettings::calibrationType calibrationType) const;

    /**
     * @brief Add current pose of robot, capture the image and calculate necessary parameters
     * prepared for calibration.
     * @param [in] value See @ref poseData.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus addPoseAndDetect(const std::vector<double>& poseData) const;

    /**
     * @brief Start to calculate the result of handeye calibration.
     * @param [out] value See @ref extrinsic.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus calculateExtrinsics(std::string& extrinsic) const;

    /**
     * @brief Only capture current image with feature recognition result for test.
     * @param [in] value See @ref colorMap.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus captureCalibrationFeatureImage(ColorMap& colorMap) const;

    /**
     * Sets the current user set by user set name. \n
     * @details Call @ref saveAllSettingsToUserSets to take effect permanently on the device.
     * @param [in] userSetName
     * @return See @ref ErrorStatus for details.
     * @see saveAllSettingsToUserSets
     */
    ErrorStatus setCurrentUserSet(const std::string& userSetName) const;

    /**
     * @brief Gets the name of the current user set.
     * @param [out] userSetName
     * @return See @ref ErrorStatus for details.
     * @see getAllUserSets
     */
    ErrorStatus getCurrentUserSet(std::string& userSetName) const;

    /**
     * @brief Gets the names of all user sets.
     * @param [out] userSetNames
     * @return See @ref ErrorStatus for details.
     * @see getCurrentUserSet
     */
    ErrorStatus getAllUserSets(std::vector<std::string>& userSetNames) const;

    /**
     * Deletes the user set by the user set name. If input name is the current user set it will
     * change the current user set to the previous index value in the user set list.
     * @details Call @ref saveAllSettingsToUserSets to take effect permanently on the device.
     * @param [in] userSetName
     * @return See @ref ErrorStatus for details.
     * @see saveAllSettingsToUserSets
     */
    ErrorStatus deleteUserSet(const std::string& userSetName) const;

    /**
     * Adds a new user set by the user set name and sets all the current device settings to it. It
     * will also change the current user set to the input value if adds success. \n
     * @details Call @ref saveAllSettingsToUserSets to take effect permanently on the device. \n
     * @param [in] userSetName
     * @return See @ref ErrorStatus for details.
     * @see saveAllSettingsToUserSets
     */
    ErrorStatus addUserSet(const std::string& userSetName) const;

private:
    MechEyeDeviceImpl* _d;
};

} // namespace api
} // namespace mmind
