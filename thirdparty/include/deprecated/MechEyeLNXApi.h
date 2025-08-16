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

namespace lnxapi {

/**
 * @brief Gets the API basic information including version and manufacturing company.
 * @return API information string.
 */
std::string MMIND_API_EXPORT getApiInformation();

/**
 * @brief Interface that is used to connect the LNX Mech-Eye device and access basic information of
 * the device.
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
     * @brief Enumerate Mech-Eye devices by the @ref MechEyeDeviceInfo identifying the
     * device.
     * @return
     *  Information on all detectable Mech-Eye devices.
     */
    static std::vector<api::MechEyeDeviceInfo> enumerateMechEyeDeviceList();

    /**
     * @brief Connect to the device by the @ref MechEyeDeviceInfo identifying a device.
     * @param [in] info the device information used to connect to the device which can be obtained
     * by @ref enumerateMechEyeDeviceList function.
     * @param [in] timeout the timeout value (ms).
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus connect(const MechEyeDeviceInfo& info, const int timeout = 10000);

    /**
     * @brief Connects to a device by the IP address and port of the device.
     * @param [in] ipAddress the IP address of the device to be connected.
     * @param [in] port the port of the device to be connected.
     * @param [in] timeout the timeout value (ms).
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus connect(const std::string& ipAddress, const int port = 5577,
                        const int timeout = 10000);

    /**
     * @brief Disconnect from the device.
     */
    void disconnect();

    /**
     * @brief Get the basic information of the connected device.
     * @param [out] info See @ref MechEyeDeviceInfo for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getDeviceInfo(api::MechEyeDeviceInfo& info) const;

    /**
     * @brief Save all parameter values to the current parameter group.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus saveAllSettingsToUserSets() const;

    /**
     * Select the parameter group to use by its name. \n
     * @param [in] userSetName
     * @return See @ref ErrorStatus for details.
     * @see saveAllSettingsToUserSets
     */
    ErrorStatus setCurrentUserSet(const std::string& userSetName) const;

    /**
     * @brief Get the name of the currently selected parameter group.
     * @param [out] userSetName
     * @return See @ref ErrorStatus for details.
     * @see getAllUserSets
     */
    ErrorStatus getCurrentUserSet(std::string& userSetName) const;

    /**
     * @brief Get the names of all available parameter groups.
     * @param [out] userSetNames
     * @return See @ref ErrorStatus for details.
     * @see getCurrentUserSet
     */
    ErrorStatus getAllUserSets(std::vector<std::string>& userSetNames) const;

    /**
     * Delete the specified parameter group. If the input name is the currently selected parameter
     * group's, the parameter group of the previous index value in the list will be selected
     * instead.
     * @param [in] userSetName
     * @return See @ref ErrorStatus for details.
     * @see saveAllSettingsToUserSets
     */
    ErrorStatus deleteUserSet(const std::string& userSetName) const;

    /**
     * Add a new parameter group, set all the current parameter values to it, and select it as the
     * current parameter group.
     * @param [in] userSetName
     * @return See @ref ErrorStatus for details.
     * @see saveAllSettingsToUserSets
     */
    ErrorStatus addUserSet(const std::string& userSetName) const;

    /**
     * @brief Perform scanning of one line and obtain the line scan image.
     * @param [out] colorMap See @ref Frame for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus captureLnx2DImage(ColorMap& colorMap) const;

    /**
     * @brief Get the acquired batch data.
     * @param [out] depth the depth value or profile value (mm)
     * @param [out] intensity the intensity value
     * @param [out] encoder the encoder value
     * @param [out] frameId the frame ID
     * @param [out] lineCount the count of frame
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxBatchRollData(float* depth, unsigned char* intensity, unsigned int* encoder,
                                    long long* frameId, int& lineCount) const;

    /**
     * @brief Get the acquired batch data.
     * @param [out] LineBatch See @ref Frame for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxBatchRollData(LineBatch& LineBatch) const;

    /**
     * @brief Stop scanning.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus stopLnxCaptureImage();

    /**
     * @brief Start scanning.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus startLnxCaptureImage();

    /**
     * @brief Set scan mode.
     * @param [in] value See @ref CaptureMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxCaptureMode(CaptureMode value) const;

    /**
     * @brief Get the current scan mode.
     * @param [out] value See @ref CaptureMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxCaptureMode(CaptureMode& value) const;

    /**
     * @brief Set the timeout value for obtaining the line scan image.
     * @param [in] time (ms)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxRawTimeout(int time) const;

    /**
     * @brief Get the current timeout value for obtaining the line scan image.
     * @param [out] time (ms)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxRawTimeout(int& time) const;

    /**
     * @brief Set the timeout value for obtaining the depth map.
     * @param [in] time (ms)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxDepthTimeout(int time) const;

    /**
     * @brief Get the current timeout value for obtaining the depth map.
     * @param [out] time (ms)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxDepthTimeout(int& time) const;

    /**
     * @brief Get the line width in the X direction.
     * @param [out] xDataWidth
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxXDataWidth(int& xDataWidth) const;

    /**
     * @brief Get the X resolution.
     * @param [out] xScale(um)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxXScale(float& xScale) const;

    /**
     * @brief Set the laser power level.
     * @param [in] powerLevel
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxLaserPowerLevel(int powerLevel) const;

    /**
     * @brief Get the current laser power level.
     * @param [out] powerLevel
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxLaserPowerLevel(int& powerLevel) const;

    /**
     * @brief Set the exposure mode.
     * @param [in] mode @ref ExposureMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScanExposureMode(ExposureMode mode) const;

    /**
     * @brief Get the current exposure mode.
     * @param [out] mode @ref ExposureMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScanExposureMode(ExposureMode& mode) const;

    /**
     * @brief Set the exposure time for the Timed exposure mode.
     * @param [in] time (μs)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxExposure(int time) const;

    /**
     * @brief Get the current exposure time for the Timed exposure mode.
     * @param [in] time (μs)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxExposure(int& time) const;

    /**
     * @brief Set the exposure sequence for the HDR exposure mode.
     * @param [in] time1 the exposure time1 (μs)
     * @param [in] time2 the exposure time2 (μs)
     * @param [in] time3 the exposure time3 (μs)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan3DHDRExposureSequence(int time1, int time2, int time3) const;

    /**
     * @brief Get the current exposure sequence for the HDR exposure mode.
     * @param [out] time1 the exposure time1 (μs)
     * @param [out] time2 the exposure time2 (μs)
     * @param [out] time3 the exposure time3 (μs)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan3DHDRExposureSequence(int& time1, int& time2, int& time3) const;

    /**
     * @brief Set the y-coordinate of the first HDR knee point.
     * @param [in] value.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxHDRDualSlope(int value) const;

    /**
     * @brief Get the current y-coordinate of the first HDR knee point.
     * @param [out] value.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxHDRDualSlope(int& value) const;

    /**
     * @brief Set the y-coordinate of the second HDR knee point.
     * @param [in] value.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxHDRTripleSlope(int value) const;

    /**
     * @brief Get the current y-coordinate of the second HDR knee point.
     * @param [out] value.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxHDRTripleSlope(int& value) const;

    /**
     * @brief Set the analog gain.
     * @param [in] gainVal See @ref ImageAnalogGain for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan3DAnalogGain(ImageAnalogGain gainVal) const;

    /**
     * @brief Get the current analog gain.
     * @param [out] gainVal See @ref ImageAnalogGain for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan3DAnalogGain(ImageAnalogGain& gainVal) const;

    /**
     * @brief Set the analog gain of LNX-8030.
     * @param [in] gainVal See @ref ImageAnalogGainFor8030 for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan3DAnalogGainFor8030(ImageAnalogGainFor8030 gainVal) const;

    /**
     * @brief Get the current analog gain of LNX-8030.
     * @param [out] gainVal See @ref ImageAnalogGainFor8030 for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan3DAnalogGainFor8030(ImageAnalogGainFor8030& gainVal) const;

    /**
     * @brief Set the digital gain.
     * @param [in] gainVal
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setScan3DDigitalGain(int gainVal) const;

    /**
     * @brief Get the current digital gain.
     * @param [out] gainVal
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getScan3DDigitalGain(int& gainVal) const;

    /**
     * @brief Set the ROI mode.
     * @param [in] mode See @ref ROIMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxROIMode(ROIMode mode) const;

    /**
     * @brief Get the current ROI mode.
     * @param [out] mode See @ref ROIMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxROIMode(ROIMode& mode) const;

    /**
     * @brief Set the trigger delay time.
     * @param [in] delay (1-2600us)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxTriggerDelay(int delay) const;

    /**
     * @brief Get the current trigger delay time.
     * @param [out] delay (1-2600us)
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxTriggerDelay(int& delay) const;

    /**
     * @brief Set the trigger source for the Depth scan mode. \n
     * Note: this function can only be used in
     * CaptureMode::Depth. It does not take effect in CaptureMode::Raw.
     * @param [in] triggerSource See @ref TriggerSource for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxDepthTriggerSource(TriggerSource triggerSource) const;

    /**
     * @brief Get the current trigger source for the Depth scan mode.
     * @param [out] triggerSource See @ref TriggerSource for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxDepthTriggerSource(TriggerSource& triggerSource) const;

    /**
     * @brief Set the line rate for the Software trigger source.
     * @param [in] rate
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxLineRate(double rate) const;

    /**
     * @brief Get the current line rate for the Software trigger source.
     * @param [out] rate
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxLineRate(double& rate) const;

    /**
     * @brief Set the maximum number of lines to be scanned.
     * @param [in] profileNum
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxProfileNum(int profileNum) const;

    /**
     * @brief Get the current maximum number of lines to be scanned.
     * @param [out] profileNum
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxProfileNum(int& profileNum) const;

    /**
     * @brief Set the encoder trigger mode.
     * @param [in] mode See @ref EncoderTriggerMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxEncoderTriggerMode(EncoderTriggerMode mode) const;

    /**
     * @brief Get the current encoder trigger mode.
     * @param [out] mode See @ref EncoderTriggerMode for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxEncoderTriggerMode(EncoderTriggerMode& mode) const;

    /**
     * @brief Set the encoder divider value. Camera frame rate = encoder signal rate * encoder
     * multiplier / encoder divider.
     * @param [in] interval
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxEncoderDivide(int interval) const;

    /**
     * @brief Get the current encoder divider value.
     * @param [out] interval
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxEncoderDivide(int& interval) const;

    /**
     * @brief Set the encoder multiplier value.
     * @param [in] multiple See @ref EncoderMultiplier for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxEncoderMultiplier(EncoderMultiplier multiple) const;

    /**
     * @brief Get the current encoder multiplier value.
     * @param [out] multiple See @ref EncoderMultiplier for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxEncoderMultiplier(EncoderMultiplier& multiple) const;

    /**
     * @brief Set the filter type.
     * @param [in] type See @ref FilterType for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxFilterType(FilterType type) const;

    /**
     * @brief Get the filter type.
     * @param [out] type See @ref FilterType for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxFilterType(FilterType& type) const;

    /**
     * @brief Set the window size for mean filtering.
     * @param [in] window See @ref MeanFilterWindow for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxMeanFilterWindow(MeanFilterWindow window) const;

    /**
     * @brief Get the current window size for mean filtering.
     * @param [out] window See @ref MeanFilterWindow for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxMeanFilterWindow(MeanFilterWindow& window) const;

    /**
     * @brief Set the window size for median filtering.
     * @param [in] window See @ref MedianFilterWindow for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxMedianFilterWindow(MedianFilterWindow window) const;

    /**
     * @brief Get the current window size for median filtering.
     * @param [out] window See @ref MedianFilterWindow for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxMedianFilterWindow(MedianFilterWindow& window) const;

    /**
     * @brief Set the maximum number of invalid points to be interpolated.
     * @param [in] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxMaxFillingPointCount(int value) const;

    /**
     * @brief Get the current maximum number of invalid points to be interpolated.
     * @param [out] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxMaxFillingPointCount(int& value) const;

    /**
     * @brief Set profile extraction strategy.
     * @param [in] value See @ref ProfileStrategy for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxProfileStrategy(ProfileStrategy value) const;

    /**
     * @brief Get the current profile extraction strategy.
     * @param [out] value See @ref ProfileStrategy for details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxProfileStrategy(ProfileStrategy& value) const;

    /**
     * @brief Set the grayscale value threshold. Grayscale values of pixels below this threshold are
     * set to zero before profile extraction.
     * @param [in] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxGrayFilterThresh(int value) const;

    /**
     * @brief Get the current grayscale value threshold.
     * @param [out] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxGrayFilterThresh(int& value) const;

    /**
     * @brief Set the minimum laser line width.
     * @param [in] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxMinStripeWidth(int value);

    /**
     * @brief Get the current minimum laser line width.
     * @param [out] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxMinStripeWidth(int& value) const;

    /**
     * @brief Set the maximum laser line width.
     * @param [in] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxMaxStripeWidth(int value);

    /**
     * @brief Get the current maximum laser line width.
     * @param [out] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxMaxStripeWidth(int& value) const;

    /**
     * @brief Set the minimum laser line intensity.
     * @param [in] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxMinStripeIntensity(int value);

    /**
     * @brief Get the current minimum laser line intensity.
     * @param [out] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxMinStripeIntensity(int& value) const;

    /**
     * @brief Set the maximum laser line intensity.
     * @param [in] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus setLnxMaxStripeIntensity(int value);

    /**
     * @brief Get the current maximum laser line intensity.
     * @param [out] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxMaxStripeIntensity(int& value) const;

    /**
     * @brief Get the current encoder value.
     * @param [out] value
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus getLnxEncoderValue(unsigned int& value) const;

private:
    MechEyeDeviceImpl* _d;
};
} // namespace lnxapi
} // namespace api
} // namespace mmind
