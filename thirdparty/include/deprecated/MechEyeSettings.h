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
#include <vector>
#include "MechEyeDataType.h"

namespace mmind {

namespace api {

/**
 * Settings used for 3D capturing.
 */
struct Scanning3DSettings
{
    /**
     * The exposure time sequence of the camera to capture the 3D images. Multiple exposures
     *will produce a HDR effect.
     * @details Long exposure is often used for scanning dark objects and short exposure is often
     *used for scanning reflective objects. \n
     * @details Getter: @ref MechEyeDevice.getScan3DExposure \n
     * @details Setter: @ref MechEyeDevice.setScan3DExposure \n
     * @note Exposure unit: ms \n
     *  Exposure minimum: 0.1 \n
     *  Exposure maximum: 99 \n
     *  Minimum Vector Size: 1 \n
     *  Maximum Vector Size: 3 \n
     */
    std::vector<double> exposureSequence;

    /**
     * Camera's gain value during scanning 3D images.
     * @details Gain is an electronic amplification of the image signal. Large gain value is needed
     * only when scanning extremely dark objects. \n
     * @details Getter: @ref MechEyeDevice.getScan3DGain \n
     * @details Setter: @ref MechEyeDevice.setScan3DGain \n
     * @note Minimum: 0 \n
     *       Maximum: 16 \n
     */
    double gain;

    /**
     * Depth map's ROI along height and width axes in camera coordinate system. Pixels locate out of
     * the ROI will be ignored.
     * @details If @ref ROI.height, @ref ROI.width, @ref ROI.x and @ref ROI.y values are all zero,
     * all pixels will be used.
     * @details Getter: @ref MechEyeDevice.getScan3DROI \n
     * @details Setter: @ref MechEyeDevice.setScan3DROI \n
     * @note  @ref ROI.x must be less than the number of columns in the image. \n
     *        @ref ROI.y must be less than the number of rows in the image. \n
     *        @ref ROI.width must be less than the width of the image. \n
     *        @ref ROI.height must be less than the height of the image. \n
     */
    ROI scan3DROI;

    /**
     * Depth image's valid range along Z-axis in camera coordinate system.
     * @details Pixels with depth value greater than the @ref DepthRange.upper and less
     * than the @ref DepthRange.lower will be ignored. \n
     * @details Getter: @ref MechEyeDevice.getDepthRange \n
     * @details Setter: @ref MechEyeDevice.setDepthRange \n
     * @note Unit: mm \n
     *       Minimum: 1 \n
     *       Maximum: 5000 \n
     */
    DepthRange depthRange;
};

/**
 * Settings used for ColorMap capturing.
 */
struct Scanning2DSettings
{
    /**
     * Option for exposure mode.
     */
    enum Scan2DExposureMode {
        Timed, ///< Fixed exposure time, recommended in the stable ambient light conditions.
        Auto,  ///< Automatic exposure time, recommended in unstable ambient light conditions.
        HDR,   ///< Generate images of high dynamic ranges showing more details with multiple
               ///< different exposures.
        Flash, ///< Use projected LED light to light up the projection area.
    };

    /**
     * Option for exposure mode.
     */
    Scan2DExposureMode exposureMode;

    /**
     * @brief The fixed camera exposure time.
     * @details Only take effect in @ref Timed mode. \n
     * @details Getter: @ref MechEyeDevice.getScan2DExposureTime \n
     * @details Setter: @ref MechEyeDevice.setScan2DExposureTime \n
     * @note Unit: ms \n
     *       Minimum: 0.1 \n
     *       Maximum: 999 \n
     */
    double exposureTime;

    /**
     * The image sharpen factor.
     * @details Take effect in all @ref Scan2DExposureMode mode. \n
     * @details Use sharpening algorithm to get sharp edge details, it may cause image noise. The
     * higher the value, the higher the image sharpness. \n
     * @details Getter: @ref MechEyeDevice.getScan2DSharpenFactor \n
     * @details Setter: @ref MechEyeDevice.setScan2DSharpenFactor \n
     * @note Minimum: 0 \n
     *       Maximum: 5 \n
     */
    double sharpenFactor;

    /**
     * The expected image gray value.
     * @details Only take effect in @ref Auto mode.
     * @details A smaller value can decrease the brightness of the image, while a larger value can
     * generate a brighter image. \n
     * @details Getter: @ref MechEyeDevice.getScan2DExpectedGrayValue \n
     * @details Setter: @ref MechEyeDevice.setScan2DExpectedGrayValue \n
     * @note Minimum: 0 \n
     *       Maximum: 255 \n
     */
    int expectedGrayValue;

    /**
     * Camera's ROI when scanning 2D images \n
     * @details Only take effect in @ref Auto and @ref HDR mode. \n
     * @details Getter: @ref MechEyeDevice.getScan2DROI \n
     * @details Setter: @ref MechEyeDevice.setScan2DROI \n
     * @note @ref ROI.x must be less than the number of columns in the image. \n
     *       @ref ROI.y must be less than the number of rows in the image. \n
     *       @ref ROI.width must be less than the width of the image. \n
     *       @ref ROI.height must be less than the height of the image. \n
     */
    ROI scan2DROI;

    /**
     * Use gray level transformation algorithm to make the image look more natural.
     * @details Only take effect in @ref HDR mode.
     * @details Getter: @ref MechEyeDevice.getScan2DToneMappingEnable \n
     * @details Setter: @ref MechEyeDevice.setScan2DToneMappingEnable \n
     */
    bool toneMappingEnable;

    /**
     * A sequence of camera exposures used for HDR capturing.
     * @details Only take effect in @ref HDR mode.
     * @details Getter: @ref MechEyeDevice.getScan2DHDRExposureSequence \n
     * @details Setter: @ref MechEyeDevice.setScan2DHDRExposureSequence \n
     * @note Exposure unit: ms  \n
     *       Exposure minimum: 0.1 \n
     *       Exposure maximum: 999 \n
     *       Minimum Vector Size: 1 \n
     *       Maximum Vector Size: 5 \n
     */
    std::vector<double> hdrExposureSequence;
};

struct PointCloudProcessingSettings
{
    /**
     * Option for cloud outlier filter.
     * @deprecated Please use @ref PointCloudProcessingSettings.outlierRemoval instead. \n
     */
    enum struct CloudOutlierFilterMode {
        Off,    ///< No outlier removal operation. Certain noisy points may exist.
        Weak,   ///< Moderate amount of outliers are removed. This mode allows having more accurate
                ///< and more complete edges.
        Normal, ///<  Most outliers are removed. Mild edge erosion might occur under certain
                ///<  circumstances.
    };

    /**
     * Option for cloud smooth filter.
     * @deprecated Please use @ref PointCloudProcessingSettings.surfaceSmoothing instead. \n
     */
    enum struct CloudSmoothMode {
        Off,    ///< No smoothing. More details of the point cloud are preserved.
        Weak,   ///< Light level surface smoothness. Noise is reduced at the expense of negligible
                ///< details.
        Normal, ///< Moderate level surface smoothness. Noise is reduced at the expense of
                ///< fine-scale details.
        Strong, ///< Advanced level surface smoothness. Noise is reduced at the expense of
                ///< moderate-scale details.
    };

    /**
     * The point cloud outlier removal level. \n
     * @details Getter: @ref MechEyeDevice.getCloudOutlierFilterMode \n
     * @details Setter: @ref MechEyeDevice.setCloudOutlierFilterMode \n
     * @deprecated Please use @ref PointCloudProcessingSettings.outlierRemoval instead. \n
     */
    CloudOutlierFilterMode outlierFilterMode;

    /**
     * Option for cloud smooth filter. \n
     * @details Getter: @ref MechEyeDevice.getCloudSmoothMode \n
     * @details Setter: @ref MechEyeDevice.setCloudSmoothMode \n
     * @deprecated Please use @ref PointCloudProcessingSettings.surfaceSmoothing instead. \n
     */
    CloudSmoothMode smoothMode;

    /**
     * Reduces the depth fluctuation in the point cloud and improves its resemblance to the actual
     * object surface.
     * Surface smoothing causes loss of object surface details. The more intense the
     * smoothing, the more details are lost.
     */
    enum struct PointCloudSurfaceSmoothing {
        Off,
        Weak,
        Normal,
        Strong,
    };

    /**
     * Removes the noise in the point cloud, thus reducing the impact on the precision and accuracy
     *  of subsequent calculation. Noise is the scattered points close to the object surface.
     */
    enum struct PointCloudNoiseRemoval {
        Off,
        Weak,
        Normal,
        Strong,
    };

    /**
     * Removes the outliers in the point cloud. Outliers are clustered points away from the
     * object point cloud.
     */
    enum struct PointCloudOutlierRemoval {
        Off,
        Weak,
        Normal,
        Strong,
    };

    /**
     * Preserves the sharpness of object edges during surface smoothing.
     */
    enum struct PointCloudEdgePreservation {
        Sharp,  ///< Preserves the sharpness of object edges as much as possible. However, the
                ///< effect of surface smoothing will be reduced.\n
        Normal, ///< Balances between edge preservation and surface smoothing.

        Smooth, ///< Does not preserve the edges. The object surface will be well smoothed,  but the
                ///< object edges will be distorted.

    };

    /**
     * The signal contrast threshold for effective pixels. Pixels with contrast less
     *than this threshold will be ignored. \n
     * @details A higher value will result in more image noise to be filtered
     *but may also cause the point cloud of dark objects to be removed. \n
     * @details Getter: @ref MechEyeDevice.getFringeContrastThreshold \n
     * @details Setter: @ref MechEyeDevice.setFringeContrastThreshold \n
     * @note Minimum: 0 \n
     *       Maximum: 100 \n
     */
    int fringeContrastThreshold;

    /**
     * The signal minimum threshold for effective pixels. Pixels with intensity less
     * than this threshold will be ignored. \n
     * @details A higher value will result in more image noise to be filtered
     * but may also cause the point cloud of dark objects to be removed. \n
     * @details Getter: @ref MechEyeDevice.getFringeMinThreshold \n
     * @details Setter: @ref MechEyeDevice.setFringeMinThreshold \n
     * @note Minimum: 0 \n
     *       Maximum: 100 \n
     */
    int fringeMinThreshold;

    /**
     * Reduces the depth fluctuation in the point cloud and improves its resemblance to the actual
     * object surface.\n
     * @details Getter: @ref MechEyeDevice.getCloudSurfaceSmoothingMode \n
     * @details Setter: @ref MechEyeDevice.setCloudSurfaceSmoothingMode \n
     * @note Surface smoothing causes loss of object surface details. The more intense the
     * smoothing, the more details are lost. \n
     */
    PointCloudSurfaceSmoothing surfaceSmoothing;

    /**
     * Removes the noise in the point cloud, thus reducing the impact on the precision and accuracy
     *  of subsequent calculation. Noise is the scattered points close to the object surface.\n
     * @details Getter: @ref MechEyeDevice.getCloudNoiseRemovalMode \n
     * @details Setter: @ref MechEyeDevice.setCloudNoiseRemovalMode \n
     * @note 1. Noise removal might remove some sharp object features. The more intense
     *  the noise removal, the more object feature might be removed. \n2. If this function removes
     * the needed object features, please reduce the intensity. However, more noise will be
     retained.
     */
    PointCloudNoiseRemoval noiseRemoval;

    /**
     * Removes the outliers in the point cloud. Outliers are clustered points away from the
     * object point cloud.\n
     * @details Getter: @ref MechEyeDevice.getCloudOutlierRemovalMode \n
     * @details Setter: @ref MechEyeDevice.setCloudOutlierRemovalMode \n
     * @note If the object point cloud contains clustered points that have
     * depth difference from other parts of the object, high intensity of outlier removal might
     * remove these points.
     */
    PointCloudOutlierRemoval outlierRemoval;

    /**
     * Preserves the sharpness of object edges during surface smoothing. \n
     * @details Getter: @ref MechEyeDevice.getCloudEdgePreservationMode \n
     * @details Setter: @ref MechEyeDevice.setCloudEdgePreservationMode \n
     * @note Sharp: Preserves the sharpness of object edges as much as possible. However, the effect
     * of surface smoothing will be reduced.\nNormal: Balances between edge preservation and surface
     * smoothing.\nSmooth: Does not preserve the edges. The object surface will be well smoothed,
     * but the object edges will be distorted.
     */
    PointCloudEdgePreservation edgePreservation;
};

/**
 * Settings used for projector during 3D capturing.              \n
 * @note Only used in Mech-Eye NANO, LOG and PRO product family. \n
 */
struct ProjectorSettings
{
    /**
     * Option for projector fringe coding mode.
     */
    enum struct FringeCodingMode {
        Fast,     ///< Fast mode has the minimum capture time.
        Accurate, ///< Accurate mode is slower but produces better depth maps than Fast mode.
    };

    /**
     * Option for projector power level.
     */
    enum struct PowerLevel {
        High,   ///< High level is ofen used for scanning dark objects.
        Normal, ///< Normal level is mostly used.
        Low,    /// Low level is used for scanning reflective objects.
    };

    /**
     * Option for projector's anti-flicker mode. Please select the option that corresponds to
     * the frequency of the power supply in use.
     */
    enum struct AntiFlickerMode {
        Off,    ///< No processing for anti-flicker.
        AC50Hz, ///< The AC frequency is 50Hz in most countries.
        AC60Hz, ///< The AC frequency in the U.S. and some Asian countries is 60Hz.
    };

    /**
     * Option for projector's fringe coding mode.
     * @note  Only support with Mech-Eye NANO and PRO series industrial 3D camera. \n
     * @details Getter: @ref MechEyeDevice.getProjectorFringeCodingMode \n
     * @details Setter: @ref MechEyeDevice.setProjectorFringeCodingMode \n
     */
    FringeCodingMode fringeCodingMode;

    /**
     * Option for projector's power level.
     * @note Only support with Mech-Eye NANO, LOG and PRO series industrial 3D camera. \n
     * @details Getter: @ref MechEyeDevice.getProjectorPowerLevel \n
     * @details Setter: @ref MechEyeDevice.setProjectorPowerLevel \n
     */
    PowerLevel powerLevel;

    /**
     * Option for anti-flicker mode that corresponds to the frequency of the power supply in
     * use.
     * @details Flicker refers to the rapid and periodical change in the intensity of artificial
     * light. This phenomenon can cause fluctuations in the depth data. Such fluctuation can be
     * reduced by adjusting the projection frequency of the structured light. \n
     * @note Only support with Mech-Eye NANO and PRO series industrial 3D camera. \n
     * @details Getter: @ref MechEyeDevice.getProjectorAntiFlickerMode \n
     * @details Setter: @ref MechEyeDevice.setProjectorAntiFlickerMode \n
     */
    AntiFlickerMode antiFlickerMode;
};

/**
 * Settings used for laser during 3D capturing.   \n
 * @note Only used in Mech-Eye Laser product family.    \n
 * @details Getter: @ref MechEyeDevice.getLaserSettings \n
 * @details Setter: @ref MechEyeDevice.setLaserSettings \n
 */
struct LaserSettings
{
    /**
     * Option for laser fringe coding mode.
     */
    enum LaserFringeCodingMode {
        Fast,     ///< Fast mode has the minimum capture time.
        Accurate, ///< Accurate mode is slower but produces better depth maps than Fast mode.
    };

    /**
     * Option for laser fringe coding mode.
     */
    LaserFringeCodingMode fringeCodingMode;

    /**
     * The laser scan field of view start position. \n
     * @details @ref frameRangeStart and @ref frameRangeEnd work together to determine the laser
     * scan field of view. Use 0~100 to represent all laser projection positions from left to
     * right under the camera's view. The frame range setting must satisfy that @ref
     * frameRangeEnd is at least 25 larger than @ref frameRangeStart. \n
     * @note Minimum: 0 \n
     *       Maximum: 100 \n
     *       @ref frameRangeEnd - @ref frameRangeStart >= 25 \n
     */
    int frameRangeStart;

    /**
     * The laser scan field of view end position. \n
     * @details @ref frameRangeStart and @ref frameRangeEnd work together to determine the laser
     * scan field of view. Use 0~100 to represent all laser projection positions from left to
     * right under the camera's view. The frame range setting must satisfy that @ref
     * frameRangeEnd is at least 25 larger than @ref frameRangeStart. \n
     * @note Minimum: 0 \n
     *       Maximum: 100 \n
     *       @ref frameRangeEnd - @ref frameRangeStart >= 25 \n
     */
    int frameRangeEnd;

    /**
     * Laser's scan partition number. \n
     * @details If the value is more than 1, the scan from start to end will be partitioned into
     * multiple parts. It is recommended to use multiple partition parts for extremely dark
     * objects. \n
     * @note Minimum: 1 \n
     *       Maximum: 4 \n
     */
    int framePartitionCount;

    /**
     * Laser Power. \n
     * @details High power is often used for scanning dark objects. Low power is often used for
     * scanning reflective objects. \n
     * @note Minimum: 50 \n
     *       Maximum: 100 \n
     */
    int powerLevel;
};

/**
 * Settings used for UHP camera capturing.   \n
 * @note Only used in Mech-Eye UHP product family.    \n
 * @details Getter: @ref MechEyeDevice.getUhpSettings \n
 * @details Setter: @ref MechEyeDevice.setUhpSettings \n
 */
struct UhpSettings
{
    /**
     * Option for UHP camera capture mode.
     */
    enum struct UhpCaptureMode {
        Camera1, ///< Camera1 mode only use camera 1.
        Camera2, ///< Camera2 mode only use camera 2.
        Merge,   ///< Merge mode use both cameras to prevent occlusions.
    };

    /**
     * Option for UHP camera fringe coding mode.
     */
    enum struct UhpFringeCodingMode {
        Fast,     ///< Fast mode has the minimum capture time.
        Accurate, ///< Accurate mode is slower but produces better depth maps than Fast mode.
    };

    /**
     * Option for UHP camera capture mode.
     * @note Only used in Mech-Eye UHP product family.    \n
     * @details Getter: @ref MechEyeDevice.getUhpCaptureMode \n
     * @details Setter: @ref MechEyeDevice.setUhpCaptureMode \n
     */
    UhpCaptureMode captureMode;

    /**
     * Option for UHP fringe coding mode.
     * @note Only used in Mech-Eye UHP product family.    \n
     * @details Getter: @ref MechEyeDevice.getUhpFringeCodingMode \n
     * @details Setter: @ref MechEyeDevice.setUhpFringeCodingMode \n
     */
    UhpFringeCodingMode fringeCodingMode;
};

/**
 * Settings used for HandEye Calibration. \n
 */
struct HandeyeCalibrationSettings
{
    /**
     * Option for calibration type.
     * @details Only take effect in process of handeye calibration.
     * @details Setter: @ref MechEyeDevice.setCalibrateType \n
     */
    enum struct calibrationType {
        EyeInHand, ///< The camera is mounted in the robot's hand
        EyeToHand, ///< The camera is fixed mountedwith respect to the robot's base.
        UNKNOWN,
    };

    /**
     * Option for board type.
     * @details Only take effect in process of handeye calibration.
     * @details Setter: @ref MechEyeDevice.setBoardType \n
     */
    enum struct boardType {
        BDB_5,
        BDB_6,
        BDB_7,
        OCB_5,
        OCB_10,
        OCB_15,
        OCB_20,
        CGB_20,
        CGB_35,
        CGB_50,
        UNKNOWN,
    };

    /**
     * The data of current robot pose which contains translation and quaternions.
     * @details Only take effect in process of handeye calibration.
     * @details Setter: @ref MechEyeDevice.addPose \n
     * @details The format is like 'Tx,Ty,Tz,Qw,Qx,Qy,Qz', the input data is split by ','. \n
     */
    std::string poseData;

    /**
     * The final result of handeye calibration.
     * @details Only take effect in process of handeye calibration.
     * @details Getter: @ref MechEyeDevice.calibrateHandEye \n
     * @details The format is like 'Tx,Ty,Tz,Qw,Qx,Qy,Qz', which is same as @ref poseData\n
     */
    std::string extrinsic;
};

} // namespace api
} // namespace mmind
