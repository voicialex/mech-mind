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
#include <functional>
#include "api_global.h"
#include "ProfilerInfo.h"
#include "ProfileData.h"
#include "UserSetManager.h"
#include "UserSet.h"

namespace mmind {

namespace eye {

class ProfilerImpl;
/**
 * @brief Describes the types of output lines of GPIO.
 */
enum class OutputLineGPIO {
    Line21 = 0,
    Line22,
    Line23,
    Line24,
    Line25,
    Line26,
    Line27,
    Line28,
};

/**
 * @brief Describes the types of output levels of GPIO.
 */
enum class OutputLevel { Low, High };
/**
 * @brief Describes the types of acquisition status.
 */
enum class AcquisitionStatus {
    AcquisitionTriggerWait, ///< @ref Profiler::startAcquisition not called.
    AcquisitionActive,      ///< @ref Profiler::startAcquisition called.
    FrameTriggerWait,       ///< @ref Profiler::triggerSoftware not called or GPIO input frame start
                            ///< signal not received.
    FrameActive, ///< @ref Profiler::triggerSoftware called or GPIO input frame start signal
                 ///< received.
};

/**
 * @brief Describes the laser profiler temperatures.
 */
struct ProfilerTemperature
{
    float controllerCpuTemperature{}; ///< The temperature (in °C) of the controller CPU.
    float sensorCpuTemperature{};     ///< The temperature (in °C) of the FPGA.
};

/**
 * @brief Describes the laser profiler's statuses.
 */
struct ProfilerStatus
{
    ProfilerTemperature temperature;
};

/**
 * @brief Operates the laser profiler.
 * Use @ref Profiler.connect to connect an available laser profiler, retrieve profile data,
 * configure parameters and so on.
 */
class MMIND_API_EXPORT Profiler
{
public:
    /**
     * @brief The type of callback function.
     * @param [out] batch The retrieved data of multiple profiles. See @ref ProfileBatch for
     * details.
     * @param [out] pUser The user data pointer.
     */
    using AcquisitionCallback = std::function<void(const ProfileBatch& batch, void* pUser)>;
    /**
     * @brief Constructor.
     */
    Profiler();

    /**
     * @brief Destructor.
     */
    ~Profiler();

    /**
     * @brief Copy constructor.
     */
    Profiler(const Profiler& other) noexcept;

    /**
     * @brief Copy assignment.
     */
    Profiler& operator=(const Profiler& other) noexcept;

    /**
     * @brief Discovers all available laser profilers and returns the list of
     * information of all available laser profilers. If a laser profiler is not successfully
     * discovered, please check the network connection and indicator lights on the laser profiler.
     * @param [in] timeoutMs The timeout period (in ms) for discovering laser profilers. If no laser
     * profilers are discovered after the timeout period has passed, this method returns an empty
     * list.
     * @return The list of information of all available laser profilers.
     */
    static std::vector<ProfilerInfo> discoverProfilers(unsigned int timeoutMs = 5000);

    /**
     * @brief Connects to a laser profiler via @ref ProfilerInfo.
     * @param [in] info Laser profiler information. Use @ref Profiler.discoverProfilers to find
     * all available laser profilers.
     * @param [in] timeoutMs The timeout for connecting a laser profiler(ms). If the execution
     * time of the connecting process is greater than the timeout, the function will immediately
     * return @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_INPUT_ERROR IP address format error.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE IP address not corresponding to an available
     *  device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Laser profiler model or
     *  firmware version not supported.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus connect(const ProfilerInfo& info, unsigned int timeoutMs = 5000);

    /**
     * @brief Saves the acquired @ref ProfileBatch data, @ref Parameter s, and @ref
     * ProfilerInfo in an MRAW format file that can be loaded as a @ref VirtualProfiler.
     *
     * @param data The acquired @ref ProfileBatch data to be saved.
     * @param fileName The name of the MRAW file to be saved. You can add a path before the name to
     * specify the path for saving the file.
     * @note Depending on the parameter settings and amount of data to be saved, it may take up to a
     * few minutes to save the virtual device file.Please ensure that the file name is encoded in
     * UTF-8 format.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid laser profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Laser profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Parse response error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR There are errors in reply.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Failed to create the virtual device file.\n
     */
    ErrorStatus saveVirtualDeviceFile(const ProfileBatch& data, const std::string& fileName);

    /**
     * @brief Connects to a laser profiler via IP address.
     * @param [in] ipAddress Valid IP address of the laser profiler, e.g. in "100.100.1.1" format.
     * @param [in] timeoutMs The timeout for connecting a laser profiler (ms). If the execution
     * time of the connecting process is greater than the timeout, the function will immediately
     * return @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_INPUT_ERROR IP address format error.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE IP address not corresponding to an
     *  available device.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Laser profiler model or
     *  firmware version not supported.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus connect(const std::string& ipAddress, unsigned int timeoutMs = 5000);

    /**
     * @brief Disconnects from the current laser profiler and releases the associated resources.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid laser profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Parse Response error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR There are errors in reply.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Laser profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus disconnect();

    /**
     * @brief Sets the time interval at which the client sends periodic heartbeat messages to the
     * profiler side. The default time interval is 10s.
     * @param [in] timeIntervalMs The time interval for periodic sending heartbeat messages in
     * milliseconds. The valid setting range is from 1s to 3600s.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_OUT_OF_RANGE_ERROR Invalid parameter input.\n
     */
    ErrorStatus setHeartbeatInterval(unsigned int intervalMs);

    /**
     * @brief Gets the basic information of the laser profiler, such as model, serial number,
     * firmware version, and IP setting, etc.
     * @param [out] See @ref ProfilerInfo for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid laser profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Laser profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     */
    ErrorStatus getProfilerInfo(ProfilerInfo& info) const;

    /**
     * @brief Gets various statuses of the laser profiler.
     * @param [out] See @ref ProfilerStatus for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Profiler disconnected.\n
     */
    ErrorStatus getProfilerStatus(ProfilerStatus& status) const;

    /**
     * @brief Gets the @ref UserSetManager of the laser profiler. @ref UserSetManager provides
     * various operations to manage all user set saved in the laser profiler, including adding and
     * deleting the user set and selecting the user set currently in effect. It is also available to
     * save all user set details to a json file and read a json file to load user set details.
     * @return See @ref UserSetManager for details.
     */
    UserSetManager& userSetManager();

    /**
     * @brief Gets the @ref UserSet currently in effect of the laser profiler. @ref UserSet can
     * access all available parameters of the laser profiler related to profile data acquisition.
     * @ref UserSet can also directly set and get parameters instead of using @ref Parameter
     * interface.
     * @return See @ref UserSet for details.
     */
    UserSet& currentUserSet();

    /**
     * @brief Retrieves a batch of the profiles. There are two ways to retrieve profile
     * data, by polling or callback. This method is only used with the polling method.
     * The number of profiles contained in a batch varies depending on the scan rate and the rate at
     * which this method is called.
     * @param [out] batch The retrieving result, it contains information including profile index,
     * encoder value intensity image, and depth data. See @ref ProfileBatch for details.
     * @param [in] timeoutMs The timeout for capturing in milliseconds. If the execution time of
     * the capturing process is greater than the timeout, the function will immediately return
     * @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid laser profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Laser profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_DATA_ERROR No profile data obtained. Some error may have
     *  occurred on the laser profiler.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_ACQUISITION_TRIGGER_WAIT Acquisition not started or stopped.\n
     */
    ErrorStatus retrieveBatchData(ProfileBatch& batch, int timeoutMs = 4000) const;

    /**
     * @brief Sends a software signal to trigger data acquisition. This method is used when no
     * external signals are input to trigger data acquisition and must be called after @ref
     * startAcquisition.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid laser profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Laser profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Parse response error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR There are errors in reply.\n
     */
    ErrorStatus triggerSoftware();

    /**
     * @brief Registers the callback function for data acquisition. There are two ways to retrieve
     * profile data, by polling or callback. This method is only used with the callback method. This
     * method must be called after @ref connect and before @ref startAcquisition. If the laser
     * profiler is in acquisition ready status, @ref stopAcquisition should be called before
     * registering a different callback function.
     * @param [in] func The callback function to be registered. When the number of retrieved
     * profiles equals to the set value of @ref ScanLineCount, the callback function will be
     * executed. See
     * @ref AcquisitionCallback for details.
     * @param [out] pUser The user data pointer.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid laser profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_CALLBACKFUNC Invalid callback function.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_BUSY The callback function registration is executed
     *  again before data acquisition is stopped.\n
     */
    ErrorStatus registerAcquisitionCallback(const Profiler::AcquisitionCallback& func, void* pUser);

    /**
     * @brief Enters the laser profiler into the acquisition ready status, where it can accept
     * trigger signals for scanning.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid laser profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Laser profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Parse response error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR There are errors in reply.\n
     */
    ErrorStatus startAcquisition();

    /**
     * @brief Exits the laser profiler from the acquisition ready status to avoid accidental
     * triggering of scanning. If a callback function is being executed when this method is called,
     * this method is not executed until the execution of the callback function is finished.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid laser profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Laser profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Parse response error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR There are errors in reply.\n
     */
    ErrorStatus stopAcquisition();

    /**
     * @brief Sets controller GPIO output value.
     * @param holdTimeMs Use this parameter to pull down the GPO after certain amount of time when
     * setting OutputLevel to High. -1 means do not pull down. @note This parameter is only
     * available on profilers with firmware version >= v2.5.0.
     * @note When setting @param holdTimeMs for several times, the last setting will be valid.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid laser profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Laser profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_NO_SUPPORT_ERROR Parameter holdTimeMs is not supported.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Parse response error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR There are errors in reply.\n
     */
    ErrorStatus setOutputForGPIO(OutputLineGPIO outputLine, OutputLevel value, int holdTimeMs = -1);

    /**
     * @brief Gets the acquisition status.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid laser profiler handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Laser profiler disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Parse response error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR: There are errors in reply.\n
     */
    ErrorStatus getAcquisitionStatus(AcquisitionStatus& status);

private:
    friend class ProfilerEvent;
    friend class InternalInterfaces;
    std::shared_ptr<ProfilerImpl> _d;
};

} // namespace eye

} // namespace mmind
