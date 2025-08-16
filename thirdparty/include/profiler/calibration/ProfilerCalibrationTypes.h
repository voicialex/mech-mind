#pragma once
#include <vector>
#include <opencv2/core/mat.hpp>
namespace mmind {
namespace eye {
constexpr int kTargetPlaneCount = 6;
constexpr int kTargetPointCount = 8;

/**
 * @brief Defines the calibration mode.
 */
enum class ProfilerCalibrationMode {
    Wide,  ///< The laser profilers are arranged in the Wide mode, i.e., side-by-side or in the
           ///< reversed direction.
    Angle, ///< The laser profilers are arranged in the Angle mode, i.e., in the opposite direction
           ///< or around a circle.
};

/**
 * @brief Specifies the axis along which one laser profiler can rotate relative to another.
 */
enum class TargetRotateAxis { X, Y, NullAxis };

/**
 * @brief Specifies the axis along which one laser profiler can translate relative to another.
 */
enum class TargetTranslateAxis { X, Y, NullAxis };

/**
 * @brief Stores the intensity image and depth map acquired by the laser profiler.
 */
struct ProfilerImage
{
    cv::Mat depth;     ///< Depth map acquired by the laser profiler.
    cv::Mat intensity; ///< Intensity image acquired by the laser profiler.
};

/**
 * @brief Defines the dimensions of the frustums of the calibration target.
 */
struct TargetSize
{
    float targetTopLength = 30; ///< The upper base length of the frustums of the calibration target
                                ///< in millimeters (mm).
    float targetBottomLength = 60; ///< The lower base length of the frustums of the calibration
                                   ///< target in millimeters (mm).
    float targetHeight =
        15; ///< The height of the frustums of the calibration target in millimeters (mm).
};

/**
 * @brief Defines the relative position of the frustums of the calibration target.
 */
struct TargetPose
{
    float translateDistance =
        83; ///< The translate distance between two frustums in millimeters (mm).
    float rotateAngleInDegree = 0; ///< The rotation angle between two frustums in degrees.
    float rotateRadius = 0; ///< The distance from the center of rotation to the midpoint of the
                            ///< frustum's height in millimeters (mm).

    ProfilerCalibrationMode mode = ProfilerCalibrationMode::Wide; ///< The calibration mode.
    TargetTranslateAxis translateAxis =
        TargetTranslateAxis::X; ///< The axis along which one frustum is translated relative to
                                ///< another.
    TargetRotateAxis rotateAxis = TargetRotateAxis::NullAxis; ///< The axis along which one frustum
                                                              ///< is rotated relative to another.
};

/**
 * @brief Defines the configuration for each laser profiler.
 */
struct DeviceInfo
{
    float dX = 0.0235; ///< The X-axis resolution of the laser profiler.
    float dY = 0.024;  ///< The Y-axis resolution of the laser profiler.
    unsigned int downsampleX =
        10; ///< The downsampling interval in the X direction. One point will be selected for every
            ///< number of points specified by this value.
    unsigned int downsampleY =
        10; ///< The downsampling interval in the Y direction. One point will be selected for every
            ///< number of points specified by this value.
    bool moveDirSign = true; ///< The motion direction of the laser profiler, with the default value
                             ///< "true" indicates the positive direction.
    cv::Point2f ROISize = {
        0.0f,
        0.0f}; ///< The size of the region of interest (ROI), measured in pixels (default: 0x0).
    cv::Point2f ROICenter = {
        0.0f, 0.0f}; ///< The center of the ROI, measured in pixels (default: at the origin).
};

/**
 * @brief Stores the motion direction and transformation matrix used during the calibration.
 */
struct CalibResultParams
{
    cv::Point3f majorMoveDirVec = cv::Point3f(
        0.0f, 0.0f, 0.0f); ///< The motion direction of the conveyor belt holding the target object
                           ///< relative to the primary laser profiler (in 3D vector format).
    cv::Point3f minorMoveDirVec = cv::Point3f(
        0.0f, 0.0f,
        0.0f); ///< The motion direction(s) of the conveyor belt holding the target object relative
               ///< to the secondary laser profiler(s) (in 3D vector format).
    cv::Matx34f matrixRT =
        cv::Matx34f::zeros(); ///< A 3x4 transformation matrix that converts coordinates from the
                              ///< point cloud of the secondary laser profiler to the coordinate
                              ///< system of the primary laser profiler.
};

/**
 * @brief Stores the calibration errors, including the reprojection errors of the primary and
 * secondary laser profilers, as well as errors related to the dual-target planes and points.
 */
struct CalibResultErrors
{
    std::vector<float> majorReprojectionErr = std::vector<float>(
        kTargetPointCount, 0.0f); ///< Reprojection errors for points in the primary laser profiler.
    std::vector<float> minorReprojectionErr = std::vector<float>(
        kTargetPointCount,
        0.0f); ///< Reprojection errors for points in the secondary laser profiler(s).
    std::vector<float> dualPlanePlaneErr =
        std::vector<float>(kTargetPlaneCount, 0.0f); ///< Plane-to-plane calibration errors between
                                                     ///< two frustums of the calibration target.
    std::vector<float> dualPlanePointErr =
        std::vector<float>(kTargetPointCount, 0.0f); ///< Plane-to-point calibration errors between
                                                     ///< two frustums of the calibration target.
    std::vector<float> dualPointPointErr =
        std::vector<float>(kTargetPointCount, 0.0f); ///< Point-to-point calibration errors between
                                                     ///< two frustums of the calibration target.
};

/**
 * @brief Stores the calibration results and errors.
 */
struct CalibResult
{
    CalibResultParams params; ///< The motion directions and transformation matrix used in the
                              ///< calibration. See @ref CalibResultParams for details.
    CalibResultErrors reprojResidual; ///< Reprojection residual errors.
};

/**
 * @brief Parameters required for stitching.
 */
struct MultiStitchParams
{
    std::string cameraModel;    ///< The model of the laser profilers.
    DeviceInfo majorDeviceInfo; ///< The device information of the primary laser profiler.
    std::vector<DeviceInfo>
        minorDeviceInfos;  ///< The device information of the secondary laser profiler.
    TargetSize targetSize; ///< The dimensions of the frustums of the calibration target.
    std::vector<TargetPose>
        targetPoses; ///< The relative position of the frustums of the calibration target.
};

/**
 * @brief Stores the stitching results of the secondary laser profiler, including the stitched image
 * and transformation matrix.
 */
struct MinorStitchResult
{
    ProfilerImage stitchResultImage; ///< The stitched image for the secondary laser profiler.
    cv::Matx34f
        matrixRT; ///< The transformation matrix (R|T) for the secondary laser profiler.camera.
};

/**
 * @brief Stores the stitching results.
 */
struct MultiStitchResult
{
    ProfilerImage majorStitchImage; ///< The stitched image for the primary laser profiler.
    std::vector<MinorStitchResult>
        minorStitchResults; ///< The stitching results of the secondary laser profiler(s).
};

/**
 * @brief Stores the stitching results of the secondary laser profiler when the laser profilers are
 * arranged in the Wide mode (i.e., side-by-side or in the reversed direction), including the
 * stitched image and bias.
 */
struct MinorStitchResultZParallel
{
    ProfilerImage stitchResultImage; ///< The stitched image for the secondary laser profiler.
    cv::Point2i bias;                ///< The bias offset of the secondary laser profiler.
};

/**
 * @brief The stitching results when the laser profilers are arranged in the Wide mode (i.e.,
 * side-by-side or in the reversed direction)
 */
struct MultiStitchResultZParallel
{
    ProfilerImage majorStitchImage; ///< The stitched image for the primary laser profiler.
    std::vector<MinorStitchResultZParallel>
        minorStitchResults; ///< The stitching results of the secondary laser profiler(s).
};

/**
 * @brief The fusion results.
 */
struct FusionResult
{
    ProfilerImage combinedImage; ///< The fused image.
    cv::Point2i majorBias;       ///< The bias offset of the primary laser profiler.
};
} // namespace eye
} // namespace mmind
