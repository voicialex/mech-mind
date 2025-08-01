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
#include "Parameter.h"

namespace mmind {

namespace eye {

// Parameters affect the images used for calculating depth data, thus affecting the quality of the
// depth map and point cloud.
namespace scanning3d_setting {

// Scanning 3D Exposure Sequence
class ExposureSequence
{
public:
    static constexpr const char* name = "Scan3DExposureSequence";

    static constexpr const char* description =
        "Set the exposure time and exposure multiplier for acquiring depth information. Usually, "
        "long exposure time is used for dark objects, and short exposure time is used for light "
        "objects. If the size of array is greater than 1, multiple exposure times must be set. "
        "Using multiple exposure times can improve the completeness of depth data but also "
        "increases processing time.\n\nNote: multiple exposure time is unavailable when "
        "\"FringeCodingMode\" is set to \"Reflective\".";

    static constexpr Parameter::Type type = Parameter::Type::_FloatArray;

    static constexpr Range<double> range() { return {0.1, 99}; }

    static constexpr int maxSize() { return 3; }

    static constexpr const char* unit = "ms";
};

// Scanning 3D Gain
class Gain
{
public:
    static constexpr const char* name = "Scan3DGain";

    static constexpr const char* description =
        "Set camera's gain value during scanning 3D images. Gain is an electronic amplification of "
        "the image signal. Large gain value is needed only when scanning extremely dark objects.";

    static constexpr Parameter::Type type = Parameter::Type::_Float;

    static constexpr Range<double> range() { return {0.0, 16.0}; }

    static constexpr const char* unit = "dB";
};

// Scanning 3D ROI
class ROI
{
public:
    static constexpr const char* name = "Scan3DROI";

    static constexpr const char* description =
        "Set the ROI for the depth map and point cloud. Points outside the selected region are "
        "removed. All values are zero if an ROI is not set.";

    static constexpr Parameter::Type type = Parameter::Type::_Roi;
};

// Depth Range
class DepthRange
{
public:
    static constexpr const char* name = "DepthRange";

    static constexpr const char* description =
        "Set the depth range in the camera reference frame. Points outside this range are removed "
        "from the depth map and point cloud.";

    static constexpr Parameter::Type type = Parameter::Type::_Range;

    static constexpr Range<int> range() { return {1, 5000}; }

    static constexpr const char* unit = "mm";
};

} // namespace scanning3d_setting

} // namespace eye

} // namespace mmind
