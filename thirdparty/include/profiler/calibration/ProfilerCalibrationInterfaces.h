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
#include <optional>
#include <opencv2/core/mat.hpp>
#include "api_global.h"
#include "profiler/calibration/MultiProfilerErrorStatus.h"
#include "profiler/calibration/ProfilerCalibrationTypes.h"

namespace mmind {
namespace eye {
class MultiProfilerCalibration;
class MMIND_API_EXPORT ProfilerCalibrationInterfaces
{
public:
    ProfilerCalibrationInterfaces();
    ~ProfilerCalibrationInterfaces();

    /**
     * @brief Sets the model of the laser profilers used in the calibration, in the form of a string
     * such as "Mech-Eye LNX-8030".
     * @param [in] cameraModel The laser profiler model.
     * @return Returns true on success and false on failure.
     */
    bool setCalibCameraModel(const std::string& cameraModel);

    /**
     * @brief Initializes the configuration for the primary laser profiler.
     * @param [in] deviceInfo The configuration for the primary laser profiler. See @ref DeviceInfo
     * for details.
     */
    void setMajorDeviceInfo(const DeviceInfo& deviceInfo);

    /**
     * @brief Initializes the configuration for the secondary laser profiler(s).
     * @param [in] deviceInfos The configuration for each secondary laser profiler. See @ref
     * DeviceInfo for details.
     */
    void setMinorDeviceInfos(const std::vector<DeviceInfo>& deviceInfos);

    /**
     * @brief Sets the dimensions of the frustums of the calibration target.
     * @param [in] targetSize The dimensions of the frustums, including the upper base length, lower
     * base length, and height. See @ref TargetSize for details.
     */
    void setCalibTargetSize(const TargetSize& targetSize);

    /**
     * @brief Defines the relative position of the frustums of the calibration target.
     * @param [in] targetPoses The position and orientation of the frustums of the calibration
     * target. See @ref TargetPose for details.
     */
    void setCalibTargetPoses(const std::vector<TargetPose>& targetPoses);

    /**
     * @brief Calculates the calibration results based on the depth maps acquired by the primary and
     * secondary laser profilers.
     * @param [in] majorDepth Depth map acquired by the primary laser profiler.
     * @param [in] minorDepths Depth map(s) acquired by secondary laser profiler(s).
     * @param [out] calibResults The calibration results and errors. See @ref CalibResult for
     * details.
     * @return See @ref MultiProfilerErrorStatus for details.
     */
    MultiProfilerErrorStatus calculateCalibration(const cv::Mat& majorDepth,
                                                  const std::vector<cv::Mat>& minorDepths,
                                                  std::vector<CalibResult>& calibResults);

    /**
     * @brief Saves the calibration data to files in the specified path.
     * @param [in] needSaveAll Specifies whether all data related to the calibration should be
     * saved. When set to "false", only the calibration configurations, results, and errors will be
     * saved. When set to "true" (the default value), the intensity images and depth maps acquired
     * by the laser profilers will also be saved.
     * @param [in] saveFolderName The path where the files should be saved. If left empty, the
     * default path will be used.
     * @return Returns true on success and false on failure.
     */
    bool saveCalibFiles(bool needSaveAll = true, const std::string& saveFolderName = "") const;

    /**
     * @brief Calculates the stitching results when the laser profilers are arranged in the Angle
     * mode (i.e., in the opposite direction or around a circle).
     * @param [in] majorImage Intensity image and depth map from the primary laser profiler.
     * @param [in] minorImages Intensity image(s) and depth map(s) from the secondary laser
     * profiler(s).
     * @param [in] calibResults The calibration results and errors used for stitching, which can be
     * either calculated or loaded from files. See @ref CalibResult for details.
     * @param [out] stitchResults The stitched intensity image and depth map, and transformation
     * result file.
     * @param [in] stitchParams Optional. If left empty, default or previously set calibration
     * parameters will be used for stitching.
     * @return See @ref MultiProfilerErrorStatus for details.
     */
    MultiProfilerErrorStatus stitchImages(
        const ProfilerImage& majorImage, const std::vector<ProfilerImage>& minorImages,
        const std::vector<CalibResult>& calibResults, MultiStitchResult& stitchResults,
        const std::optional<MultiStitchParams>& stitchParams = std::nullopt);

    /**
     * @brief Calculates the stitching results when the laser profilers are arranged in the Wide
     * mode (i.e., side-by-side or in the reversed direction).
     * @param [in] majorImage Intensity image and depth map from the primary laser profiler.
     * @param [in] minorImages Intensity image(s) and depth map(s) from the secondary laser
     * profiler(s).
     * @param [in] calibResults The calibration results and errors used for stitching, which can be
     * either calculated or loaded from files. See @ref CalibResult for details.
     * @param [out] stitchResults The stitched intensity image and depth map and transformation
     * result file.
     * @param [in] stitchParams Optional. If left empty, default or previously set calibration
     * parameters will be used for stitching.
     * @return See @ref MultiProfilerErrorStatus for details.
     */
    MultiProfilerErrorStatus stitchImagesForZParallel(
        const ProfilerImage& majorImage, const std::vector<ProfilerImage>& minorImages,
        const std::vector<CalibResult>& calibResults, MultiStitchResultZParallel& stitchResults,
        const std::optional<MultiStitchParams>& stitchParams = std::nullopt);

    /**
     * @brief Performs fusion based on the stitching results when the laser profilers are arranged
     * in the Wide mode (i.e., side-by-side or in the reversed direction).
     * @param [out] fusionResult The fused intensity image and depth map, and the coordinates of the
     * upper-left pixels of the intensity image and depth map acquired by the primary laser
     * profiler.
     * @param [in] fusionFlag Optional. Specifies whether to perform fusion. If left empty, fusion
     * will be performed.
     * @return See @ref MultiProfilerErrorStatus for details.
     */
    MultiProfilerErrorStatus imageFusionForZParallel(FusionResult& fusionResult,
                                                     const std::vector<bool>& fusionFlag = {});

    /**
     * @brief Performs fusion based on the stitching results when the laser profilers are arranged
     * in the Wide mode (i.e., side-by-side or in the reversed direction).
     * @param [in] majorStitchImage The intensity image and depth map acquired by the primary laser
     * profiler and converted to the coordinate system of the secondary laser profiler.
     * @param [in] minorStitchResults The intensity image and depth map acquired by the secondary
     * laser profiler(s) and converted to the coordinate system of the primary laser profiler.
     * @param [in] majorBias The bias of the primary laser profiler relative to the laser profiler
     * used as the reference in stitching.
     * @param [out] fusionResult The fused intensity image and depth map, and the coordinates of the
     * upper-left pixels of the intensity image and depth may acquired by the primary laser
     * profiler.
     * @param [in] fusionFlag Optional. Specifies whether to perform fusion. If left empty, fusion
     * will be performed.
     * @return See @ref MultiProfilerErrorStatus for details.
     */
    MultiProfilerErrorStatus imageFusionForZParallel(
        const ProfilerImage& majorStitchImage,
        const std::vector<MinorStitchResultZParallel>& minorStitchResults,
        const cv::Point2i& majorBias, FusionResult& fusionResult,
        const std::vector<bool>& fusionFlag = {});

    /**
     * @brief Saves the stitching results to files in the specified path.
     * @param [in] saveFolderName The path where the files should be saved. If left empty, the
     * default path will be used.
     * @return Returns true on success and false on failure.
     */
    bool saveStitchFiles(const std::string& saveFolderName = "") const;

    /**
     * @brief Saves the stitching results to files in the specified path when the laser profilers
     * are arranged in the Wide mode (i.e., side-by-side or in the reversed direction).
     * @param [in] saveFolderName The path where the files should be saved. If left empty, the
     * default path will be used.
     * @return Returns true on success and false on failure.
     */
    bool saveStitchFilesForZParallel(const std::string& saveFolderName = "") const;

    /**
     * @brief Loads the calibration data from the files in the specified path, typically used before
     * stitching starts.
     * @param [in] loadFolderName The path of the files storing the calibration data to be loaded.
     * @param [in] needLoadAll Specifies whether all data related to the calibration should be
     * loaded. When set to "false", only the calibration configurations, results, and errors will be
     * loaded. When set to "true" (the default value), the intensity images and depth maps acquired
     * by the laser profilers will also be loaded.
     * @return See @ref MultiProfilerErrorStatus for details.
     */
    MultiProfilerErrorStatus loadCalibProperties(const std::string& loadFolderName = "",
                                                 bool needLoadAll = false);
    /**
     * @brief Gets the current calibration results, which can be used for stitching.
     * @return Returns the calibration results and errors. See @ref CalibResult for details.
     */
    std::vector<CalibResult> getCurrentCalibResults() const;

    /**
     * @brief Gets the major device information.
     * This function retrieves the information of the primary device used in the calibration
     * or processing pipeline.
     * @return Returns a `DeviceInfo` object containing the details of the major device.
     */
    DeviceInfo getMajorDeviceInfo() const;

    /**
     * @brief Gets the target size.
     * This function retrieves the size of the calibration target.
     * @return Returns a `TargetSize` object representing the size of the target.
     */
    TargetSize getTargetSize() const;

    /**
     * @brief Gets the camera model identifier.
     * This function retrieves the model name of the profiler.
     * @return Returns a string containing the camera model name.
     */
    std::string getCameraModel() const;

    /**
     * @brief Gets the minor devices' information.
     * This function retrieves a list of information for all minor devices in the system.
     * @return Returns a vector of `DeviceInfo` containing details of the minor devices.
     */
    std::vector<DeviceInfo> getMinorDeviceInfos() const;

    /**
     * @brief Gets the calibration target poses.
     * This function retrieves the spatial positions and orientations of the calibration target
     * captured during the calibration process.
     * @return Returns a vector of `TargetPose` containing the captured pose data.
     */
    std::vector<TargetPose> getTargetPoses() const;

    /**
     * @brief Constructs a new `ProfilerCalibrationInterfaces` object with specified parameters.
     * This constructor initializes the calibration interface with the provided camera model,
     * device information, target size, and target poses.
     * @param [in] cameraModel A string representing the camera model (e.g., "Mech-Eye LNX-8030").
     * @param [in] majorDeviceInfo A structure containing the configuration parameters of the
     * major device.
     * @param [in] minorDeviceInfos A vector of `DeviceInfo` structures containing configuration
     * data for each minor device.
     * @param [in] targetSize A structure defining the size of the calibration target.
     * @param [in] targetPoses A vector of `TargetPose` structures describing the positions and
     * orientations of the calibration targets.
     */
    ProfilerCalibrationInterfaces(const std::string& cameraModel, const DeviceInfo& majorDeviceInfo,
                                  const std::vector<DeviceInfo>& minorDeviceInfos,
                                  const TargetSize& targetSize,
                                  const std::vector<TargetPose>& targetPoses);

private:
    std::unique_ptr<MultiProfilerCalibration> calibrationInstance;
};

} // namespace eye

} // namespace mmind
