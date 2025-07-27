#pragma once
#include "CommonTypes.h"
#include "area_scan_3d_camera/Frame3D.h"
#include "area_scan_3d_camera/Frame2DAnd3D.h"
#include "area_scan_3d_camera/Camera.h"
#include "api_global.h"

namespace mmind {
namespace eye {
/**
 * @brief Transforms the reference frame of a point cloud.
 * @param [in] coordinateTransformation The rigid body transformations from the original reference
 * frame to the new reference frame. Refer to @ref FrameTransformation for more details.
 * @param [in] originalPointCloud The @ref PointCloud whose reference frame is to be transformed.
 * @return The @ref PointCloud after reference frame transformation.
 * @note This method must be called after @ref Camera.capture3D and @ref
 * Frame3D.getUntexturedPointCloud so that the @ref PointCloud data is available.
 */

MMIND_API_EXPORT PointCloud transformPointCloud(const FrameTransformation& coordinateTransformation,
                                                const PointCloud& originalPointCloud);

/**
 * @brief Transforms the reference frame of a point cloud with normals.
 * @param [in] coordinateTransformation The rigid body transformations from the original reference
 * frame to the new reference frame. Refer to @ref FrameTransformation for more details.
 * @param [in] originalPointCloud The @ref PointCloud whose reference frame is to be transformed.
 * @return The @ref PointCloudWithNormals after reference frame transformation.
 * @note This method must be called after @ref Camera.capture3D and @ref
 * Frame3D.getUntexturedPointCloud so that the @ref PointCloud data is available.
 */
MMIND_API_EXPORT PointCloudWithNormals transformPointCloudWithNormals(
    const FrameTransformation& coordinateTransformation, const PointCloud& originalPointCloud);

/**
 * @brief Transforms the reference frame of a textured point cloud.
 * @param [in] coordinateTransformation The rigid body transformations from the original reference
 * frame to the new reference frame. Refer to @ref FrameTransformation for more details.
 * @param [in] originalTexturedPointCloud The @ref TexturedPointCloud whose reference frame is to be
 * transformed.
 * @return The @ref TexturedPointCloud after reference frame transformation.
 * @note This method must be called after @ref Camera.capture2DAnd3D and @ref
 * Frame2DAnd3D.getTexturedPointCloud so that the @ref TexturedPointCloud data is available.
 */
MMIND_API_EXPORT TexturedPointCloud
transformTexturedPointCloud(const FrameTransformation& coordinateTransformation,
                            const TexturedPointCloud& originalTexturedPointCloud);

/**
 * @brief Transforms the reference frame of a textured point cloud with normals.
 * @param [in] coordinateTransformation The rigid body transformations from the original reference
 * frame to the new reference frame. Refer to @ref FrameTransformation for more details.
 * @param [in] originalTexturedPointCloud The @ref TexturedPointCloud whose reference frame is to be
 * transformed.
 * @return The @ref TexturedPointCloudWithNormals after reference frame transformation.
 * @note This method must be called after @ref Camera.capture2DAnd3D and @ref
 * Frame2DAnd3D.getTexturedPointCloud so that the @ref TexturedPointCloud data is available.
 */
MMIND_API_EXPORT TexturedPointCloudWithNormals
transformTexturedPointCloudWithNormals(const FrameTransformation& coordinateTransformation,
                                       const TexturedPointCloud& originalTexturedPointCloud);
/**
 * @brief Obtains the rigid body transformations of the custom reference frame of a camera.
 * The custom reference frame can be adjusted using the "Custom Reference Frame" tool in Mech-Eye
 * Viewer. The rigid body transformations are automatically calculated after the settings in this
 * tool have been applied. The "Custom Reference Frame" tool is recommended as the GUI allows you to
 * adjust the reference frame easily and conveniently.
 * Alternatively, you can use the rotation and translation methods in @ref FrameTransformation to
 * define the transformations manually.
 * @param [in] camera The @ref Camera handle.
 * @return The rigid body transformations from the camera reference frame to the custom reference
 *frame. Refer to @ref FrameTransformation for more details.
 */
MMIND_API_EXPORT FrameTransformation getTransformationParams(Camera& camera);
} // namespace eye
} // namespace mmind
