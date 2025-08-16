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

// parameters affect the quality of the 2D image.
namespace scanning2d_setting {

// Scanning 2D Exposure Mode
class ExposureMode
{
public:
    static constexpr const char* name = "Scan2DExposureMode";

    static constexpr const char* description =
        "Sets the exposure mode for capturing the 2D image. Timed: Sets a single exposure time. "
        "Usually used in stable lighting conditions. Auto: The exposure time is automatically "
        "adjusted. Usually used in varying lighting conditions. HDR: Sets multiple exposure times "
        "and merge the images. Usually used for objects with various colors or textures. Flash: "
        "Uses the projector for supplemental light. Usually used in dark environments.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Timed,
        Auto,
        HDR,
        Flash,
    };
};

// Scanning 2D Exposure Time
class ExposureTime
{
public:
    static constexpr const char* name = "Scan2DExposureTime";

    static constexpr const char* description =
        "Sets the exposure time for capturing the 2D image. Usually, long exposure time is used in "
        "dark environments, and short exposure time is used in bright environments. "
        "\"ExposureTime\" is unavailable when \"ExposureMode\" is not set to \"Timed\".";

    static constexpr Parameter::Type type = Parameter::Type::_Float;

    static constexpr Range<float> range() { return {0.1, 999}; }

    static constexpr const char* unit = "ms";
};

class SharpenFactor
{
public:
    static constexpr const char* name = "Scan2DSharpenFactor";

    static constexpr const char* description =
        "Use sharpening algorithm to get sharp edge details, it may cause image noise. The higher "
        "the setting value, the higher the image sharpness.";

    static constexpr Parameter::Type type = Parameter::Type::_Float;

    static constexpr double defaultValue{0.0};

    static constexpr Range<float> range() { return {0.0, 5.0}; }
};

class ExpectedGrayValue
{
public:
    static constexpr const char* name = "Scan2DExpectedGrayValue";

    static constexpr const char* description =
        "This parameter affects the brightness of the 2D image. Increase the value if the 2D image "
        "is too dark and decrease if too bright. \"ExpectedGrayValue\" is unavailable when "
        "\"ExposureMode\" is not set to \"Auto\".";

    static constexpr Parameter::Type type = Parameter::Type::_Int;

    static constexpr Range<int> range() { return {0, 255}; }
};

class ToneMappingEnable
{
public:
    static constexpr const char* name = "Scan2DToneMappingEnable";

    static constexpr const char* description =
        "This function can make the image look more natural. If the 2D image appears very "
        "different from the actual objects, please enable this function. \"ToneMappingEnable\" is "
        "unavailable when \"ExposureMode\" is not set to \"HDR\".";

    static constexpr Parameter::Type type = Parameter::Type::_Bool;
};

class AutoExposureROI
{
public:
    static constexpr const char* name = "Scan2DROI";

    static constexpr const char* description =
        "If an auto-exposure ROI is set, the exposure time is adjusted based on the lighting, "
        "object colors, etc., in this region. Please select the area where the target objects are "
        "located and avoid including irrelevant objects as much as possible. \"AutoExposureROI\" "
        "is unavailable when \"ExposureMode\" is not set to \"Auto\".";

    static constexpr Parameter::Type type = Parameter::Type::_Roi;
};

class HDRExposureSequence
{
public:
    static constexpr const char* name = "Scan2DHDRExposureSequence";

    static constexpr const char* description =
        "Set multiple exposure times, and the captured images are merged to generate a 2D image "
        "that retains more details in the highlights and shadows. \"HDRExposureSequence\" is "
        "unavailable when \"ExposureMode\" is not set to \"HDR\".";

    static constexpr Parameter::Type type = Parameter::Type::_FloatArray;

    static constexpr Range<float> range() { return {0.1, 999}; }

    static constexpr int maxSize() { return 5; }

    static constexpr const char* unit = "ms";
};

// Depth Source camera Exposure Mode
class DepthSourceExposureMode
{
public:
    static constexpr const char* name = "Scan2DPatternRoleExposureMode";

    static constexpr const char* description =
        "Sets the exposure mode for capturing the 2D images (depth source). The images are used in "
        "hand-eye calibration, checking intrinsic parameters and setting ROI. Timed: Set a single "
        "fixed exposure time. Usually used in stable lighting conditions. Flash: Use the projector "
        "for supplemental light. Usually used in dark environments.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Timed,
        Flash,
    };
};

// Depth Source camera Exposure Time
class DepthSourceExposureTime
{
public:
    static constexpr const char* name = "Scan2DPatternRoleExposureTime";

    static constexpr const char* description =
        "Sets the exposure time for capturing the 2D images (depth source). Usually, long exposure "
        "time is used in dark environments, and short exposure time is used in bright "
        "environments.";

    static constexpr Parameter::Type type = Parameter::Type::_Float;

    static constexpr Range<float> range() { return {0.1, 999}; }

    static constexpr const char* unit = "ms";
};

// 2D Camera Flash Acquisition Mode
class FlashAcquisitionMode
{
public:
    static constexpr const char* name = "Scan2DFlashAcquisitionMode";

    static constexpr const char* description =
        "Selects the mode of acquiring the 2D image when using the projector for supplemental "
        "light. \n\n* Fast: The 2D image is acquired as part of the 3D data, providing a faster "
        "acquisition speed. Recommended for applications that use capture2DAnd3D() and require "
        "short cycle time. \n* Responsive: The 2D image is acquired independently, ensuring the "
        "correctness of the 2D image acquired by capture2D(). However, it takes longer to acquire "
        "both 2D and 3D data. Recommended for applications that acquire the 2D and 3D data "
        "separately.\n\nNote: \n* If the \"Fast\" mode is used with capture2D() and capture3D(), "
        "as the acquisition of the 2D image requires all 3D data to be acquired first, capture2D() "
        "must be called after capture3D(). If the scene changes after capture3D() is called, the "
        "2D image acquired by capture2D() will not match the actual scene.\n* When \"Fast\" is "
        "selected, the brightness of the 2D image is affected by the \"Scan3DExposureSequence\" "
        "and \"ProjectorPowerLevel\" / \"LaserPowerLevel\" parameters.\n* When \"Responsive\" is "
        "selected, the brightness of the 2D image is affected by the \"ProjectorPowerLevel\" / "
        "\"LaserPowerLevel\" parameters. \n* For the models that do not have this parameter, their "
        "acquisition mode in the flash exposure mode is the same as the \"Fast\" mode.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Responsive,
        Fast,
    };
};

// 2D Camera Flash Exposure Time
class FlashExposureTime
{
public:
    static constexpr const char* name = "Scan2DFlashExposureTime";

    static constexpr const char* description =
        "When \"FlashAcquisitionMode\" is set to \"Responsive\", set the exposure time for "
        "capturing the 2D image. Usually, long exposure time is used in dark environments, and "
        "short exposure time is used in bright environments. \n\nNote:  For the DEEP and LSR "
        "series, this parameter must be set to a multiple of 4, and the minimum value that can be "
        "set is 8 ms. The entered value is automatically adjusted.";

    static constexpr Parameter::Type type = Parameter::Type::_Float;

    static constexpr Range<float> range() { return {0.1, 99}; }

    static constexpr const char* unit = "ms";
};

// Scanning 2D Gain
class Gain
{
public:
    static constexpr const char* name = "Scan2DGain";

    static constexpr const char* description =
        "Set camera's gain value during scanning 2D images. Gain is an electronic amplification of "
        "the image signal. Large gain value is needed only when scanning extremely dark objects.";

    static constexpr Parameter::Type type = Parameter::Type::_Float;

    static constexpr Range<double> range() { return {0.0, 16.0}; }

    static constexpr const char* unit = "dB";
};

} // namespace scanning2d_setting

} // namespace eye

} // namespace mmind
