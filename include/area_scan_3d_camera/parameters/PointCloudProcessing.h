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

// Process the generated point cloud. Please perform image capturing again after adjusting the
// parameters to see the result.
namespace pointcloud_processing_setting {

// Point Cloud Surface Smoothing
class SurfaceSmoothing
{
public:
    static constexpr const char* name = "PointCloudSurfaceSmoothing";

    static constexpr const char* description =
        "Reduces the depth fluctuation in the point cloud and improves its resemblance to the "
        "actual object surface. Surface smoothing causes loss of object surface details. The more "
        "intense the smoothing, the more details are lost.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Off,
        Weak,
        Normal,
        Strong,
    };
};

// Point Cloud Noise Removal
class NoiseRemoval
{
public:
    static constexpr const char* name = "PointCloudNoiseRemoval";

    static constexpr const char* description =
        "Removes the noise in the point cloud, thus reducing the impact on the precision and "
        "accuracy of subsequent calculation. Noise is the scattered points close to the object "
        "surface. Noise removal might remove some sharp object features. The more intense the "
        "noise removal, the more object features might be removed. If this function removes the "
        "needed object features, please reduce the intensity. However, more noise will be "
        "retained.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Off,
        Weak,
        Normal,
        Strong,
    };
};

// Point Cloud Outlier Removal
class OutlierRemoval
{
public:
    static constexpr const char* name = "PointCloudOutlierRemoval";

    static constexpr const char* description =
        "Removes the outliers in the point cloud. Outliers are clustered points away from the "
        "object point cloud. If the object point cloud contains clustered points that have depth "
        "difference from other parts of the object, high intensities of outlier removal might "
        "remove these points.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Off,
        Weak,
        Normal,
        Strong,
    };
};

// Point Cloud Gap Filling
class GapFilling
{
public:
    static constexpr const char* name = "PointCloudGapFilling";

    static constexpr const char* description =
        "Fill in the gaps in the point cloud so that the object's surface features are more "
        "complete.\n\nNote:\n* This parameter is only available when \"FringeCodingMode\" in the "
        "\"mmind::eye::projector_setting\" namespace is set to \"Translucent\".\n* More intense "
        "gap filling "
        "fills more missing points but may also distort object edges.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Off,
        Weak,
        Normal,
        Strong,
    };
};

// Point Cloud Edge Preservation
class EdgePreservation
{
public:
    static constexpr const char* name = "PointCloudEdgePreservation";

    static constexpr const char* description =
        "Preserves the sharpness of object edges during surface smoothing. Sharp: Preserves the "
        "sharpness of object edges as much as possible. However, the effect of surface smoothing "
        "will be reduced. Normal: Balances between edge preservation and surface smoothing. "
        "Smooth: Does not preserve the edges.The object surface will be well smoothed, but the "
        "object edges will be distorted.\n\nNote: \"EdgePreservation\" is unavailable when "
        "\"FringeCodingMode\" in the \"mmind::eye::projector_setting\" namespace is set to "
        "\"Translucent\".";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Sharp,
        Normal,
        Smooth,
    };
};

class EnableDistortionCorrection
{
public:
    static constexpr const char* name = "EnableDistortionCorrection";

    static constexpr const char* description =
        "Set this parameter to true to enable distortion correction.\n\nNote:\n* Enabling "
        "distortion correction lowers the "
        "acquisition speed.\n* \"EnableDistortionCorrection\" is unavailable when "
        "\"FringeCodingMode\" is set to \"Translucent\" or \"Reflective\".";

    static constexpr Parameter::Type type = Parameter::Type::_Bool;
};

class DistortionCorrection
{
public:
    static constexpr const char* name = "DistortionCorrection";

    static constexpr const char* description =
        "Adjust the intensity of distortion correction.\n\nNote:\n* A \"DistortionCorrection\" too "
        "large may result in adverse effects. Please acquire data again after adjusting the "
        "parameter to check its influence on the depth map and point cloud.\n* "
        "\"DistortionCorrection\" is unavailable when \"FringeCodingMode\" is set to "
        "\"Translucent\" or \"Reflective\".";

    static constexpr Parameter::Type type = Parameter::Type::_Int;

    static constexpr Range<int> range() { return {1, 10}; }

    static constexpr const char* unit = "";
};

// Stripe Contrast Threshold
class FringeContrastThreshold
{
public:
    static constexpr const char* name = "FringeContrastThreshold";

    static constexpr const char* description =
        "If the level of noise is still high after adjusting Outlier Removal and Noise Removal, "
        "please increase the value of this parameter. However, the points of dark objects might be "
        "lost.";

    static constexpr Parameter::Type type = Parameter::Type::_Int;

    static constexpr Range<int> range() { return {1, 100}; }

    static constexpr const char* unit = "";
};

// Minimum Fringe Intensity Threshold
class FringeMinThreshold
{
public:
    static constexpr const char* name = "FringeMinThreshold";

    static constexpr const char* description =
        "Set the signal minimum threshold for effective pixels. Pixels with intensity less than "
        "this threshold will be ignored. A higher value will result in more image noise to be "
        "filtered but may also cause the point cloud of dark objects to be removed.\n\nNote: "
        "\"FringeMinThreshold\" is unavailable when \"FringeCodingMode\" is set to \"Reflective\".";

    static constexpr Parameter::Type type = Parameter::Type::_Int;

    static constexpr Range<int> range() { return {1, 100}; }

    static constexpr const char* unit = "";
};

class EdgeArtifactRemoval
{
public:
    static constexpr const char* name = "EdgeArtifactRemoval";

    static constexpr const char* description = "";

    static constexpr Parameter::Type type = Parameter::Type::_Bool;
};

} // namespace pointcloud_processing_setting
} // namespace eye
} // namespace mmind
