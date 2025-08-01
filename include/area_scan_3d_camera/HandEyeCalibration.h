#pragma once
#include <cmath>
#include "Camera.h"
#include "CameraProperties.h"

namespace mmind {

namespace eye {

class MMIND_API_EXPORT HandEyeCalibration
{
public:
    /**
     * @details Only takes effect in process of hand-eye calibration. Option for camera mounting
     * mode.
     */
    enum struct CameraMountingMode {
        EyeInHand, ///< The camera is mounted in the robot's hand.
        EyeToHand, ///< The camera is fixed mounted with respect to the robot's base.
    };

    /**
     * @details Only takes effect in process of hand-eye calibration. Option for board model.
     */
    enum struct CalibrationBoardModel {
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
    };

    /**
     * @brief Defines rigid body transformations, including translation vector and
     * quaternion vector.
     */
    struct Transformation
    {
        Transformation() = default;
        Transformation(double x, double y, double z, double qW, double qX, double qY, double qZ)
            : x(x), y(y), z(z), qW(qW), qX(qX), qY(qY), qZ(qZ)
        {
        }

        std::string toString() const
        {
            char buff[256] = {0};
            snprintf(buff, sizeof(buff), "%lf,%lf,%lf,%lf,%lf,%lf,%lf", x, y, z, qW, qX, qY, qZ);
            return buff;
        }
        double x{}; // Translation 3*1 vector robot pose unit: mm; extrinsic unit: m.
        double y{};
        double z{};
        double qW{1}; // Quaternion 4*1 vector
        double qX{};
        double qY{};
        double qZ{};
    };
    /**
     * @brief Sets the board model and camera mounting mode for this calibration.
     * @param [in] camera The camera handle.
     * @param [in] mountingMode The camera mounting mode. See @ref CameraMountingMode for details.
     * @param [in] boardModel The calibration board model. See @ref CalibrationBoardModel for
     * details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus initializeCalibration(Camera& camera, CameraMountingMode mountingMode,
                                      CalibrationBoardModel boardModel);

    /**
     * @brief Adds current pose of the robot, captures the image, and calculates necessary
     * parameters prepared for calibration.
     * @param [in] camera The camera handle.
     * @param [in] poseData The current pose of the robot. See @ref Transformation for details.
     * @param [out] color2DImage The current 2D image with feature recognition result. See @ref
     * Color2DImage for details.
     * @param [in] timeoutMs The timeout for connecting a camera in ms unit. If the execution
     * time of the connecting process is greater than the timeout, the function will immediately
     * return @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus addPoseAndDetect(Camera& camera, const Transformation& poseData,
                                 Color2DImage& color2DImage, unsigned int timeoutMs = 10000);

    /**
     * @brief Starts to calculate the result of hand-eye calibration.
     * @param [in] camera The camera handle.
     * @param [out] cameraToBase The result of hand-eye calibration. See @ref Transformation for
     * details.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus calculateExtrinsics(Camera& camera, Transformation& cameraToBase);

    /**
     * @brief Only captures current image with feature recognition result for test.
     * @param [in] camera The camera handle.
     * @param [out] testResult The current 2D image with feature recognition result. See @ref
     * Color2DImage for details.
     * @param [in] timeoutMs The timeout for connecting a camera in ms unit. If the execution
     * time of the connecting process is greater than the timeout, the function will immediately
     * return @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR.
     * @return See @ref ErrorStatus for details.
     */
    ErrorStatus testRecognition(Camera& camera, Color2DImage& testResult,
                                unsigned int timeoutMs = 10000);
};

} // namespace eye

} // namespace mmind
