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
#include "area_scan_3d_camera/Camera.h"
#include "profiler/Profiler.h"

namespace mmind {

namespace eye {

class MMIND_API_EXPORT InternalInterfaces
{
public:
    /**
     * @brief Get the depth image to main camera transformation for UHP cameras.
     */
    static ErrorStatus getCam3DPoseInMain(const Camera& camera, Transformation& transformation);

    /**
     * @brief Get the camera user name
     */
    static ErrorStatus getCameraUserName(const Camera& camera, std::string& cameraUserName);

    /**
     * @brief Get the @ref Frame2D from depth source camera.
     */
    static ErrorStatus captureDepthSource2D(const Camera& camera, Frame2D& frame2d,
                                            unsigned int timeoutMs = 5000);
    /**
     * @brief Simultaneously captures a 2D frame from depth source camera and a single 3D Frame.
     */
    static ErrorStatus captureDepthSource2DAnd3D(const Camera& camera, Frame2DAnd3D& frame2d3d,
                                                 unsigned int timeoutMs = 5000);

    /**
     * @brief Get the profiler user name
     */
    static ErrorStatus getProfilerUserName(const Profiler& profiler, std::string& cameraUserName);

    /**
     * @brief Get the Retrieval net speed
     *
     * @return net speed in Kb/s
     */
    static double getRetrievalNetSpeed(const Frame2D& frame);
    static double getRetrievalNetSpeed(const Frame3D& frame);
    static double getRetrievalNetSpeed(const Frame2DAnd3D& frame);

    /**
     * @brief Save virtual profiler related information to files in the folder provided.
     */
    static ErrorStatus saveVirtualProfilerInfo(const Profiler& profiler,
                                               const std::string& folderPath);

    /**
     * @brief Zip folder provided to a virtual profiler file.
     */
    static bool zipVirtualProfilerFile(const std::string& zipFilePath,
                                       const std::string& folderPath);

    /**
     * @brief Unzip virtual profiler file provided to the folder provided.
     */
    static bool unzipVirtualProfilerFile(const std::string& zipFilePath,
                                         const std::string& folderPath);
    /**
     * @brief Determines whether precision compensation is supported.
     *
     * @return true if precision compensation is supported, otherwise false.
     */
    static bool isPrecisionCompensationSupported(const Profiler& profiler);

    /**
     * @brief Determines whether the precision compensation table is available.
     *
     * @return true if the precision compensation table can be obtained, otherwise false.
     */
    static bool isPrecisionCompensationTableAvailable(const Profiler& profiler);

    /**
     * @brief Enables or disables precision compensation for the profiler.
     *
     * @param profiler The profiler instance for which precision compensation should be configured.
     * @param enable Set to true to enable precision compensation, or false to disable it.
     */
    static ErrorStatus enablePrecisionCompensation(const Profiler& profiler, bool enable);

    /**
     * @brief Checks whether precision compensation is enabled for the profiler.
     *
     * @param profiler The profiler instance to check.
     * @return True if precision compensation is enabled, false otherwise.
     */
    static bool isPrecisionCompensationEnabled(const Profiler& profiler);
};

} // namespace eye

} // namespace mmind
