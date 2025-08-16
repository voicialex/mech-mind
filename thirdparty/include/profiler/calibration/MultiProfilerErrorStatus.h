#pragma once
#include <string>
#include <unordered_map>
namespace mmind {
namespace eye {
class MultiProfilerErrorStatus
{
public:
    enum ErrorCode {
        MMIND_STATUS_SUCCESS = 0, //< Success.

        // error code for calibration
        MMIND_STATUS_EMPTY_DEPTH_IMAGE = -1,
        MMIND_STATUS_PLANE_SEGMENTATION_FAILURE = -2,
        MMIND_STATUS_NONE_PARALLEL_PLANES = -3,
        MMIND_STATUS_OVERMUCH_PARALLEL_PLANES = -4,
        MMIND_STATUS_REORDER_PLANES_FAILURE = -5,
        MMIND_STATUS_REPROJECT_ERROR_HIGH = -6,
        MMIND_STATUS_INVALID_ROI = -7,

        // error code for stitcher
        MMIND_STATUS_NON_STANDARD_RESOLUTION = -8,
        MMIND_STATUS_CAMERA_MODEL_ERROR = -9,
        MMIND_STATUS_INVALID_BOUNDARY_VALUE = -10,
        MMIND_STATUS_FUSION_POINT_EXCEED_MEMORY = -11,
        MMIND_STATUS_MULTI_STITCH_CAMERA_MODEL_EMPTY = -12,
        MMIND_STATUS_MULTI_STITCH_PARAMS_EMPTY = -13,
        MMIND_STATUS_PARAMS_ARE_NOT_CHECKED = -14,
        MMIND_STATUS_POINT_CLOUDS_EMPTY = -15,
        MMIND_STATUS_NO_IMAGE_AVAILABLE = -16,
        MMIND_STATUS_DEPTH_INTENSITY_IMAGE_SIZE_UNMATCHED = -17,
        MMIND_STATUS_DEPTH_IMAGE_TYPE_WRONG = -18,
        MMIND_STATUS_INTENSITY_IMAGE_TYPE_WRONG = -19,
        MMIND_STATUS_MAJOR_AND_MINOR_RESOLUTION_INCONSISTENCY = -20,
        MMIND_STATUS_MULTI_STITCH_PARAMS_SIZE_INCONSISTENCY = -21,
        MMIND_STATUS_MULTI_FUSION_PARAMS_SIZE_INCONSISTENCY = -22,
        MMIND_STATUS_MINOR_INFO_UNMATCHED_IN_FUSION = -23,
        MMIND_STATUS_EXTRACT_IMAGE_INFO_ERROR = -24,
        MMIND_STATUS_EMPTY_BIASES_IN_FUSION = -25,
        MMIND_STATUS_IMAGE_AND_MASK_SIZE_UNMATCHED_IN_FUSION = -26,

        // error code for checking validity
        MMIND_STATUS_INVALID_PARAM = -27,
        MMIND_STATUS_INVALID_POSITIVE_PARAM = -28,
        MMIND_STATUS_INVALID_IMAGE_CONFIG_PAIR = -29,
        MMIND_STATUS_INVALID_IMAGE_RESULT_PAIR = -30,
        MMIND_STATUS_EMPTY_MULTISYSTEM_CONFIG = -31,
        MMIND_STATUS_INVALID_ROTATION_MATRIX = -32,
        MMIND_STATUS_INVALID_MOVE_DIR_VECTOR = -33,
        MMIND_STATUS_INVALID_MOVE_DIR_VECTOR_Y = -34,
        MMIND_STATUS_INVALID_TOP_LENGTH = -35,
        MMIND_STATUS_INVALID_ROTATE_RADIUS_ANGLE_MODE = -36,
        MMIND_STATUS_INVALID_ROTATE_RADIUS_WIDE_MODE = -37,
        MMIND_STATUS_INVALID_ROTATE_ANGLE_ANGLE_MODE = -38,
        MMIND_STATUS_INVALID_ROTATE_ANGLE_WIDE_MODE = -39,
        MMIND_STATUS_INVALID_TRANSLATE_DISTANCE_ANGLE_MODE = -40,
        MMIND_STATUS_INVALID_ROTATE_AXIS_ANGLE_MODE = -41,
        MMIND_STATUS_INVALID_ROTATE_AXIS_WIDE_MODE = -42,
        MMIND_STATUS_INVALID_TRANSLATE_AXIS_ANGLE_MODE = -43,
        MMIND_STATUS_INVALID_TRANSLATE_AXIS_WIDE_MODE = -44,
        MMIND_STATUS_INVALID_DOWNSAMPLE = -45,
        MMIND_STATUS_INVALID_DEPTH_GROUPID = -46,
        MMIND_STATUS_INVALID_CONFIG_GROUPID = -47,
        MMIND_STATUS_INVALID_RESULT_GROUPID = -48,

        // error code for IO
        MMIND_STATUS_INPUT_ERROR = -49,

        // error code for load calib files
        MMIND_STATUS_CONFIG_LOAD_FAILURE = -50,
        MMIND_STATUS_RESULT_LOAD_FAILURE = -51,
        MMIND_STATUS_EVALS_LOAD_FAILURE = -52,
        MMIND_STATUS_DEPTH_LOAD_FAILURE = -53,
    };

    enum ErrorSource { System, MajorDevice, MinorDevice };

    MultiProfilerErrorStatus() = default;
    MultiProfilerErrorStatus(ErrorCode code, const std::string& message)
        : errorCode(code), errorDescription(message)
    {
    }

    bool isOK() const
    {
        return errorCode == MMIND_STATUS_SUCCESS ||
               errorCode == MMIND_STATUS_NON_STANDARD_RESOLUTION;
    }

    void setErrorCodeAndDescription(const ErrorCode& code)
    {
        this->errorCode = code;
        this->errorDescription = errorInfoMap[code];
    }

    void setErrorCodeAndDescription(const ErrorCode& code, const std::string& paraName)
    {
        this->errorCode = code;
        this->errorDescription = errorInfoMap[code] + paraName;
    }

    unsigned int groupID = 0;
    ErrorSource errorSource = System;
    ErrorCode errorCode = MMIND_STATUS_SUCCESS;
    std::string errorDescription = "Success";

private:
    // description for calibration
    std::string emptyImageDescription =
        "Failed to access the specified image. Please check the file path and ensure the file "
        "exists.";
    std::string planeSegmentationFailureDescription =
        "The number of sampling points in the point cloud available for plane segmentation is too "
        "few. Please check the input depth maps and ensure that they only contain the six feature "
        "planes of the calibration target.";
    std::string noneParallelPlanesDescription =
        "Plane 1 and 2 were not found. Please check the input depth maps and ensure that they only "
        "contain the six feature planes of the calibration target.";
    std::string overmuchParallelPlanesDescription =
        "More than two parallel surfaces were identified. Please ensure the depth map only "
        "contains the six feature planes of the calibration target. If two laser profilers share a "
        "common field of view, laser light interference may cause this error; in that case, "
        "the 'Trigger Delay' parameter should be set for one laser profiler.";
    std::string reorderPlanesFailureDescription =
        "Failed to order the feature planes. Please ensure that each lateral face forms a 45° "
        "angle with both the upper and lower bases, and that the depth map contains only the six "
        "feature planes of the calibration target.";
    std::string reprojectErrorHighDescription =
        "The reprojection error is excessive. Please verify the input parameters: (1) The depth "
        "maps should only contain the six feature planes of the calibration target; (2) Ensure the "
        "resolutionX/Y in deviceInfo (algorithm parameter) matches the actual specifications, and "
        "confirm the targetSize parameters correspond with practical usage; (3) Excessive noise in "
        "captured data may exist - appropriately increasing downsampleX/Y (downsampling intervals "
        "along X/Y axes) might alleviate this issue, with recommended values in [10, 40]; (4) "
        "Contaminants like dust, scratches, or reflective areas on the calibration target could "
        "interfere with accuracy; (5) Verify the calibration target meets required mechanical "
        "machining precision; (6) Check whether the camera contour mode parameters align with "
        "recommended values.";
    std::string invalidROIDescription =
        "The ROI (Region of Interest) of the real-time collected image should match the ROI of the "
        "device information. Please check if the ROI in depthImageInfo and dualProfilerConfig is "
        "consistent.";
    // description for stitcher
    std::string nonStandardResolutionDescription =
        "In stitching, the user-input resolution in the X direction is an LNX non-standard "
        "resolution. Please verify that the output matches expectations. This might occur because "
        "the user rescaled the image before stitching. If errors or unexpected results arise "
        "during stitching, first check whether the resolution input is correct.";
    std::string cameraModelErrorDescription = "Please input the model of the laser profiler.";
    std::string invalidBoundaryValueDescription =
        "In stitching, the image boundary detection during the resampling step returned invalid "
        "values. Please check the input parameters, especially whether the RT matrix is incorrect.";
    std::string fusionPointExceedMemoryDescription =
        "During stitching, the point cloud fusion process required more computing space than the "
        "available capacity.";
    std::string multiStitchCameraModelEmptyDescription =
        "In stitching, the 'camera model' parameter list is empty.";
    std::string multiStitchParamsEmptyDescription =
        "In stitching, the 'stitching parameters' list is empty.";
    std::string multiStitchParamsSizeInconsistencyDescription =
        "In stitching, the length of the 'stitching parameters' list does not match the length of "
        "the 'camera model' parameter list.";
    std::string multiFusionParamsSizeInconsistencyDescription =
        "The length of fusionFlag or fusionParam variable input during fusion does not match the "
        "length of the minor camera image.";
    std::string stitchParamsAreNotCheckedDescription =
        "Before calling the stitching function, the parameter check function must be called first.";
    std::string pointCloudsEmptyDescription = "The input point cloud data was empty.";
    std::string depthImageTypeWrongDescription = "The input image type was not CV_32F or CV_32F1.";
    std::string resolutionInconsistencyDescription =
        "During stitching, the X-axis and Y-axis resolutions must be identical, and the "
        "resolutions for both axes should be the same across the primary and secondary laser "
        "profilers.";
    std::string noImageAvailableDescription =
        "The intensity image and depth map in the input data were unavailable.";
    std::string depthAndIntensityImageSizeWrongDescription =
        "During stitching, the dimensions of the depth map and intensity map from the same camera "
        "do not match.";
    std::string intensityImageTypeWrongDescription =
        "The input intensity image type was not CV_8UC1.";
    std::string biasesEmptyDescription = "In fusion, the 'image bias' parameter list is empty.";
    std::string minorInfoSizeErrorDescription =
        "In fusion, the lengths of the ‘image data’ list and the ‘image bias’ parameter list are "
        "inconsistent.";
    std::string imageExtractInfomationErrorDescription =
        "In fusion, when extracting the fused image from the fused large image, the computed ROI "
        "exceeds the original image size. The possible reason is incorrect image size or image "
        "bias input into the function. This interface is not open to the public, so the risk of "
        "this error occurring is minimal.";
    std::string imageAndMaskSizeUnmatchedDescription =
        "In fusion, the sizes of the valid point mask image and the original image do not match.";
    // description for checking validity
    std::string invalidParamDescription =
        "The input parameters were empty or contain invalid values such as infinite or Nan.";
    std::string invalidPositiveParamDescription =
        "Input parameters should be positive finite values.";
    std::string invalidImageConfigPairDescription =
        "The lengths of minorImageInfos and multiProfilerConfig input parameters are inconsistent; "
        "please check the relevant inputs.";
    std::string invalidImageResultPairDescription =
        "The lengths of minorImageInfos and multiProfilerResult input parameters are inconsistent; "
        "please check the relevant inputs.";
    std::string emptyMultiProfilerConfigDescription =
        "The multiProfilerConfig input parameter is empty; please check the relevant inputs.";
    std::string invalidRotateMatrixDescription = "The rotation matrix is not an orthogonal matrix.";
    std::string invalidMoveDirVecDescription = "The motion direction vector is not a unit vector.";
    std::string invalidMoveDirVecYDescription =
        "The Y component of the motion direction vector should be a non-negative number.";
    std::string invalidTopLengthDescription =
        "The upper base length of the frustum should be smaller than the lower base length.";
    std::string invalidRotateRadiusInAngleModeDescription =
        "In the Angled mode, the value of rotateRadius should be greater than half of the frustum "
        "height.";
    std::string invalidRotateRadiusInWideModeDescription =
        "In the Wide mode, the value of rotateRadius should be 0.";
    std::string invalidRotateAngleInAngleModeDescription =
        "In the Angled mode, when one frustum is rotated relative to another along the Y axis, the "
        "included angle between frustum axes (rotateAngleInDegree) should be within [-180, 180]; "
        "when one frustum is rotated relative to another along the X axis, the included angle "
        "between frustum axes (rotateAngleInDegree) should be within (-90, 90).";
    std::string invalidRotateAngleInWideModeDescription =
        "In the Wide mode, the rotation angle between two frustums (rotateAngleInDegree) should be "
        "0.";
    std::string invalidTranslateDistanceInAngleModeDescription =
        "In the Angled mode, the translate distance between two frustums (translateDistance) "
        "should be 0.";
    std::string invalidRotateAxisInAngleModeDescription =
        "In the Angled mode, the axis along which one frustum is rotated relative to another "
        "(targetRotateAxis) should not be NullAxis.";
    std::string invalidRotateAxisInWideModeDescription =
        "In the Wide mode, the axis along which one frustum is rotated relative to another "
        "(targetRotateAxis) should be NullAxis.";
    std::string invalidTranslateAxisInAngleModeDescription =
        "In the Angled mode, the axis along which one frustum is translated relative to another "
        "(targetTranslateAxis) should be NullAxis.";
    std::string invalidTranslateAxisInWideModeDescription =
        "In the Wide mode, the axis along which one frustum is translated relative to another "
        "(targetTranslateAxis) should not be NullAxis.";
    std::string invalidDownSampleDescription =
        "The input downsampling parameters (downsampleX and downsampleY) should be integers within "
        "the range [1, 40].";
    std::string invalidDepthGroupIDDescription =
        "The groupID of members in the xxx data structure should match the index of their "
        "position.";
    std::string invalidConfigGroupIDDescription =
        "The groupID of each dualProfilerConfig in multiProfilerConfig should correspond to its "
        "index in the vector (a data structure). For example, if there are three members in "
        "multiProfilerConfig named dualProfilerConfig0 to dualProfilerConfig2, the groupID of "
        "dualProfilerConfig0 should be 0, dualProfilerConfig1 should be 1, and dualProfilerConfig2 "
        "should be 2.";
    std::string invalidResultGroupIDDescription =
        "The groupID of each dualProfilerResult in multiProfilerResults should correspond to its "
        "index in the vector (a data structure). For example, if there are three members in "
        "multiProfilerResults named dualProfilerResult0 to dualProfilerResult2, the groupID of "
        "dualProfilerResult0 should be 0, dualProfilerResult1 should be 1, and dualProfilerResult2 "
        "should be 2.";

    std::unordered_map<ErrorCode, std::string> errorInfoMap = {
        // for calibration
        {MMIND_STATUS_EMPTY_DEPTH_IMAGE, emptyImageDescription},
        {MMIND_STATUS_PLANE_SEGMENTATION_FAILURE, planeSegmentationFailureDescription},
        {MMIND_STATUS_NONE_PARALLEL_PLANES, noneParallelPlanesDescription},
        {MMIND_STATUS_OVERMUCH_PARALLEL_PLANES, overmuchParallelPlanesDescription},
        {MMIND_STATUS_REORDER_PLANES_FAILURE, reorderPlanesFailureDescription},
        {MMIND_STATUS_REPROJECT_ERROR_HIGH, reprojectErrorHighDescription},
        {MMIND_STATUS_INVALID_ROI, invalidROIDescription},
        // for stitcher
        {MMIND_STATUS_NON_STANDARD_RESOLUTION, nonStandardResolutionDescription},
        {MMIND_STATUS_CAMERA_MODEL_ERROR, cameraModelErrorDescription},
        {MMIND_STATUS_INVALID_BOUNDARY_VALUE, invalidBoundaryValueDescription},
        {MMIND_STATUS_FUSION_POINT_EXCEED_MEMORY, fusionPointExceedMemoryDescription},
        {MMIND_STATUS_MULTI_STITCH_CAMERA_MODEL_EMPTY, multiStitchCameraModelEmptyDescription},
        {MMIND_STATUS_MULTI_STITCH_PARAMS_EMPTY, multiStitchParamsEmptyDescription},
        {MMIND_STATUS_MULTI_STITCH_PARAMS_SIZE_INCONSISTENCY,
         multiStitchParamsSizeInconsistencyDescription},
        {MMIND_STATUS_PARAMS_ARE_NOT_CHECKED, stitchParamsAreNotCheckedDescription},
        {MMIND_STATUS_POINT_CLOUDS_EMPTY, pointCloudsEmptyDescription},
        {MMIND_STATUS_DEPTH_IMAGE_TYPE_WRONG, depthImageTypeWrongDescription},
        {MMIND_STATUS_NO_IMAGE_AVAILABLE, noImageAvailableDescription},
        {MMIND_STATUS_DEPTH_INTENSITY_IMAGE_SIZE_UNMATCHED,
         depthAndIntensityImageSizeWrongDescription},
        {MMIND_STATUS_INTENSITY_IMAGE_TYPE_WRONG, intensityImageTypeWrongDescription},
        {MMIND_STATUS_MAJOR_AND_MINOR_RESOLUTION_INCONSISTENCY, resolutionInconsistencyDescription},
        {MMIND_STATUS_EMPTY_BIASES_IN_FUSION, biasesEmptyDescription},
        {MMIND_STATUS_MINOR_INFO_UNMATCHED_IN_FUSION, minorInfoSizeErrorDescription},
        {MMIND_STATUS_EXTRACT_IMAGE_INFO_ERROR, imageExtractInfomationErrorDescription},
        {MMIND_STATUS_IMAGE_AND_MASK_SIZE_UNMATCHED_IN_FUSION,
         imageAndMaskSizeUnmatchedDescription},
        {MMIND_STATUS_MULTI_FUSION_PARAMS_SIZE_INCONSISTENCY,
         multiFusionParamsSizeInconsistencyDescription},

        // for checking validity
        {MMIND_STATUS_INVALID_PARAM, invalidParamDescription},
        {MMIND_STATUS_INVALID_POSITIVE_PARAM, invalidPositiveParamDescription},
        {MMIND_STATUS_INVALID_IMAGE_CONFIG_PAIR, invalidImageConfigPairDescription},
        {MMIND_STATUS_INVALID_IMAGE_RESULT_PAIR, invalidImageResultPairDescription},
        {MMIND_STATUS_EMPTY_MULTISYSTEM_CONFIG, emptyMultiProfilerConfigDescription},
        {MMIND_STATUS_INVALID_ROTATION_MATRIX, invalidRotateMatrixDescription},
        {MMIND_STATUS_INVALID_MOVE_DIR_VECTOR, invalidMoveDirVecDescription},
        {MMIND_STATUS_INVALID_MOVE_DIR_VECTOR_Y, invalidMoveDirVecYDescription},
        {MMIND_STATUS_INVALID_TOP_LENGTH, invalidTopLengthDescription},
        {MMIND_STATUS_INVALID_ROTATE_RADIUS_ANGLE_MODE, invalidRotateRadiusInAngleModeDescription},
        {MMIND_STATUS_INVALID_ROTATE_RADIUS_WIDE_MODE, invalidRotateRadiusInWideModeDescription},
        {MMIND_STATUS_INVALID_ROTATE_ANGLE_ANGLE_MODE, invalidRotateAngleInAngleModeDescription},
        {MMIND_STATUS_INVALID_ROTATE_ANGLE_WIDE_MODE, invalidRotateAngleInWideModeDescription},
        {MMIND_STATUS_INVALID_TRANSLATE_DISTANCE_ANGLE_MODE,
         invalidTranslateDistanceInAngleModeDescription},
        {MMIND_STATUS_INVALID_ROTATE_AXIS_ANGLE_MODE, invalidRotateAxisInAngleModeDescription},
        {MMIND_STATUS_INVALID_ROTATE_AXIS_WIDE_MODE, invalidRotateAxisInWideModeDescription},
        {MMIND_STATUS_INVALID_TRANSLATE_AXIS_ANGLE_MODE,
         invalidTranslateAxisInAngleModeDescription},
        {MMIND_STATUS_INVALID_TRANSLATE_AXIS_WIDE_MODE, invalidTranslateAxisInWideModeDescription},
        {MMIND_STATUS_INVALID_DOWNSAMPLE, invalidDownSampleDescription},
        {MMIND_STATUS_INVALID_DEPTH_GROUPID, invalidDepthGroupIDDescription},
        {MMIND_STATUS_INVALID_CONFIG_GROUPID, invalidConfigGroupIDDescription},
        {MMIND_STATUS_INVALID_RESULT_GROUPID, invalidResultGroupIDDescription},
    };
};
} // namespace eye
} // namespace mmind
