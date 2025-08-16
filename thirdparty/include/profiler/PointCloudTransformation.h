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
#include "CommonTypes.h"
#include "profiler/Profiler.h"
#include "api_global.h"
namespace mmind {
namespace eye {
/**
 * @brief Obtains the rigid body transformations of the custom reference frame of a laser profiler.
 * The custom reference frame can be adjusted using the "Custom Reference Frame" tool in Mech-Eye
 * Viewer. The rigid body transformations are automatically calculated after the settings in this
 * tool have been applied. The "Custom Reference Frame" tool is recommended as the GUI allows you to
 * adjust the reference frame easily and conveniently.
 * Alternatively, you can use the rotation and translation methods in @ref FrameTransformation to
 * define the transformations manually.
 * @param [in] profiler The @ref profiler handle.
 * @return The rigid body transformations from the laser profiler reference frame to the custom
 * reference frame. Refer to @ref FrameTransformation for more details.
 */
MMIND_API_EXPORT FrameTransformation getTransformationParams(Profiler& profiler);

/**
 * @brief Transforms the reference frame of a point cloud.
 * @param [in] coordinateTransformation The rigid body transformations from the original reference
 * frame to the new reference frame. Refer to @ref FrameTransformation for more details.
 * @param [in] originalPointCloud The @ref ProfileBatch::UntexturedPointCloud whose reference frame
 * is to be transformed.
 * @return The @ref ProfileBatch::UntexturedPointCloud after reference frame transformation.
 */
MMIND_API_EXPORT ProfileBatch::UntexturedPointCloud transformPointCloud(
    const FrameTransformation& coordinateTransformation,
    const ProfileBatch::UntexturedPointCloud& originalPointCloud);

/**
 * @brief Transforms the reference frame of a textured point cloud.
 * @param [in] coordinateTransformation The rigid body transformations from the original reference
 * frame to the new reference frame. Refer to @ref FrameTransformation for more details.
 * @param [in] originalTexturedPointCloud The @ref TexturedPointCloud whose reference frame is to be
 * transformed.
 * @return The @ref TexturedPointCloud after reference frame transformation.
 */
MMIND_API_EXPORT ProfileBatch::TexturedPointCloud transformTexturedPointCloud(
    const FrameTransformation& coordinateTransformation,
    const ProfileBatch::TexturedPointCloud& originalTexturedPointCloud);

} // namespace eye
} // namespace mmind
