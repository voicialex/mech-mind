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
#include "CameraProperties.h"
#include "Frame2DAnd3D.h"

namespace mmind {

namespace eye {

/**
 * @brief Uses a gray scale image as a mask to transform a depth map into a point cloud, the valid
 * points in the point cloud only contain the mapped depth for each valid pixel in the mask.
 * @note The unit of the translational component of the @ref CameraIntrinsics is mm. If the unit of
 * the point cloud is m, the point cloud cannot be correctly masked. Therefore, please set the
 * unit of the point cloud to mm using @ref Camera::setPointCloudUnit before calling this method.
 * @param [in] depthMap The input depth map. See @ref DepthMap for details.
 * @param [in] textureValidMask A mask image of the same size as the camera's 2D image, with valid
 * points set to 255 and invalid points to 0.
 * @param [in] intrinsics The camera intrinsics. See @ref CameraIntrinsics for details.
 * @param [out] pointCloud The output point cloud. See @ref PointCloud for details.
 * @return
 *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
 *  @ref ErrorStatus.MMIND_STATUS_INVALID_INPUT_ERROR Input error.\n
 */
MMIND_API_EXPORT ErrorStatus getPointCloudAfterMapping(const DepthMap& depthMap,
                                                       const GrayScale2DImage& textureValidMask,
                                                       const CameraIntrinsics& intrinsics,
                                                       PointCloud& pointCloud);

/**
 * @brief Uses a gray scale image as a mask to transform a depth map and a texture image into a
 * textured point cloud, the valid points in the point cloud only contain the mapped depth for each
 * valid pixel in the mask.
 * @note The unit of the translational component of the @ref CameraIntrinsics is mm. If the unit of
 * the point cloud is m, the point cloud cannot be correctly masked and textured. Therefore, please
 * set the unit of the point cloud to mm using @ref Camera::setPointCloudUnit before calling this
 * method.
 * @param [in] depthMap The input depth map. See @ref DepthMap for details.
 * @param [in] textureValidMask A mask image of the same size as the camera's 2D image, with valid
 * points set to 255 and invalid points to 0.
 * @param [in] texture The input texture image. See @ref Color2DImage for details.
 * @param [in] intrinsics The camera intrinsics. See @ref CameraIntrinsics for details.
 * @param [out] pointCloud The output textured point cloud. See @ref TexturedPointCloud for details.
 * @return
 *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
 *  @ref ErrorStatus.MMIND_STATUS_INVALID_INPUT_ERROR Input error.\n
 */
MMIND_API_EXPORT ErrorStatus getPointCloudAfterMapping(const DepthMap& depthMap,
                                                       const GrayScale2DImage& textureValidMask,
                                                       const Color2DImage& texture,
                                                       const CameraIntrinsics& intrinsics,
                                                       TexturedPointCloud& pointCloud);

} // namespace eye

} // namespace mmind
