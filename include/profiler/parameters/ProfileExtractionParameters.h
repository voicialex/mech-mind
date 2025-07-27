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
#include "Parameter.h"

namespace mmind {
namespace eye {
namespace profile_extraction {

class MinGrayscaleValue
{
public:
    static constexpr const char* name = "MinGrayscaleValue";

    static constexpr const char* description =
        "Set the minimum grayscale value of the valid pixels in the raw image. Pixels with "
        "grayscale values smaller than this value will not participate in profile "
        "extraction.\n\nNote: \n* The minimum value of \"Min Grayscale Value\" is affected by "
        "\"Digital Gain\".";

    static constexpr Parameter::Type type = Parameter::Type::_Int;
};

class MinSpotIntensity
{
public:
    static constexpr const char* name = "MinSpotIntensity";

    static constexpr const char* description =
        "This parameter is only effective for firmware 2.2.1 and below."
        "For firmware 2.3.0 and above, adjustment of this parameter does not take effect.\n"
        "Set the minimum intensity for the spots. Spots with intensity values smaller than this "
        "value will be excluded. The intensity of a spot is the average grayscale value of all the "
        "valid pixels in the pixel column of the laser line.\nThe spots of laser lines produced by "
        "stray light or interreflection usually have low intensities. Setting an appropriate "
        "minimum intensity can remove these spots.\n\nNote: The minimum value of \"Min Spot "
        "Intensity\" is affected by \"Min Grayscale Value\".";

    static constexpr Parameter::Type type = Parameter::Type::_Int;
};

class MaxSpotIntensity
{
public:
    static constexpr const char* name = "MaxSpotIntensity";

    static constexpr const char* description =
        "This parameter is only effective for firmware 2.2.1 and below."
        "For firmware 2.3.0 and above, adjustment of this parameter does not take effect.\n"
        "Set the maximum intensity for the spots. Spots with intensity values greater than this "
        "value will be excluded. The intensity of a spot is the average grayscale value of all the "
        "valid pixels in the pixel column of the laser line.\nSetting an appropriate maximum "
        "intensity can remove abnormally bright spots produced by specular reflection.\n\nNote: "
        "The minimum value of \"Max Spot Intensity\" is affected by \"Min Grayscale Value\".";

    static constexpr Parameter::Type type = Parameter::Type::_Int;
};

class MinLaserLineWidth
{
public:
    static constexpr const char* name = "MinLaserLineWidth";

    static constexpr const char* description =
        "Set the minimum width for the laser lines. If the width of a pixel column in a laser line "
        "is smaller than this value, this pixel column in this laser line does not participate in "
        "profile extraction.\nLaser line width is a property of each pixel column in a laser line. "
        "It is equal to the number of valid pixels in such a pixel column.\nSetting appropriate "
        "minimum and maximum widths can exclude the laser lines produced by stray light or "
        "interreflection, which are usually too wide or too narrow.";

    static constexpr Parameter::Type type = Parameter::Type::_Int;
};

class MaxLaserLineWidth
{
public:
    static constexpr const char* name = "MaxLaserLineWidth";

    static constexpr const char* description =
        "Set the maximum width for the laser lines. If the width of a pixel column in a laser line "
        "is greater than this value, this pixel column in this laser line does not participate in "
        "profile extraction.\nLaser line width is a property of each pixel column in a laser line. "
        "It is equal to the number of valid pixels in such a pixel column.\nSetting appropriate "
        "minimum and maximum widths can exclude the laser lines produced by stray light or "
        "interreflection, which are usually too wide or too narrow.";

    static constexpr Parameter::Type type = Parameter::Type::_Int;
};

class SpotSelection
{
public:
    static constexpr const char* name = "SpotSelection";

    static constexpr const char* description =
        "If a pixel column contains multiple laser lines, the laser line used for profile "
        "extraction is selected according to the value of this parameter.\nStrongest: selects the "
        "laser line with the highest intensity for profile extraction.\nNearest: selects the laser "
        "line closest to the laser profiler for profile extraction.\nFarthest: selects the laser "
        "line farthest from the laser profiler for profile extraction.\nInvalid: regards the pixel "
        "column as invalid. The profile therefore has a gap. Usually used for complex situations "
        "where selection is difficult to make.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Strongest,
        Nearest,
        Farthest,
        Invalid,
    };
};

class EdgeSelection
{
public:
    static constexpr const char* name = "EdgeSelection";

    static constexpr const char* description =
        "Select the location for extracting the profile in each laser line.\nTop edge: extracts "
        "the profile from the top edge of the laser line.\nCenter: extracts the profile from the "
        "center of the laser line.\nBottom edge: extracts the profile from the bottom edge of the "
        "laser line.\nIf the target object is a transparent/translucent object, such as glue, you "
        "can select \"Top edge\".";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value { Center, TopEdge, BottomEdge };
};

class MinSharpness
{
public:
    static constexpr const char* name = "MinSharpness";

    static constexpr const char* description =
        "Set the minimum sharpness of the laser lines. Sharpness is the clearness of the edges of "
        "a laser line. Increasing this parameter can exclude the laser lines produced by stray "
        "light or interreflection, which are usually too dark and blurry.";

    static constexpr Parameter::Type type = Parameter::Type::_Int;
};

class BrightnessAdjustment
{
public:
    static constexpr const char* name = "BrightnessAdjustment";

    static constexpr const char* description =
        "Adjusts the brightness of the intensity image. A greater value of this parameter results "
        "in a brighter intensity image. The initial brightness of the intensity image is "
        "Scale_1_0_0.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Scale_0_5_0, /* 0.5  */
        Scale_0_7_5, /* 0.75 */
        Scale_1_0_0, /* 1.0  */
        Scale_1_5_0, /* 1.5  */
        Scale_2_0_0, /* 2.0  */
    };
};

} // namespace profile_extraction
} // namespace eye
} // namespace mmind
