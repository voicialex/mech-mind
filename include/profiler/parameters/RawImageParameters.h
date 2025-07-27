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

namespace brightness_settings {

class ExposureMode
{
public:
    static constexpr const char* name = "ExposureMode";

    static constexpr const char* description =
        "Select the exposure mode for acquiring the raw image based on the texture and color of "
        "the target object.\nTimed: suitable for target objects of a single texture or color. "
        "\nHDR: suitable for target objects with various textures or colors.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        HDR,
        Timed,
    };
};

class ExposureTime
{
public:
    static constexpr const char* name = "ExposureTime";

    static constexpr const char* description =
        "The function of this parameter depends on the the value of \"ExposureMode\".\n\nIf "
        "\"ExposureMode\" is set to \"Timed\": Set the exposure time for acquiring the raw image "
        "in this parameter. Exposure time affects the brightness and width of the laser lines in "
        "the raw image, as well as the \"MaxScanRate\" of the laser profiler.\nLonger exposure "
        "time results in brighter and wider laser lines and lower \"MaxScanRate\". Shorter "
        "exposure time results in darker and narrower laser lines and higher "
        "\"MaxScanRate\".\n\nIf \"ExposureMode\" is set to \"HDR\": Set the total time for three "
        "exposures in this parameter. Then, adjust \"HdrExposureTimeProportion1\" and "
        "\"HdrExposureTimeProportion2\" to change the lengths of each exposure.\n\nTo enhance the "
        "\"MaxScanRate\", decrease this parameter and increase \"AnalogGain\".\nIf this parameter "
        "has reached its maximum value, but the laser lines are still too dark, increase "
        "\"AnalogGain\".\nIf this parameter has reached its minimum value, but the laser lines are "
        "still too bright, decrease \"LaserPower\".";

    static constexpr Parameter::Type type = Parameter::Type::_Int;

    static constexpr const char* unit = "us";
};

class HdrExposureTimeProportion1
{
public:
    static constexpr const char* name = "HdrExposureTimeProportion1";

    static constexpr const char* description =
        "Adjust the proportion of the first exposure time in the total exposure time of the HDR "
        "exposure mode. The value of this parameter is usually greater than the value of "
        "\"HdrExposureTimeProportion2\".";

    static constexpr Parameter::Type type = Parameter::Type::_Float;

    static constexpr const char* unit = "%";
};

class HdrExposureTimeProportion2
{
public:
    static constexpr const char* name = "HdrExposureTimeProportion2";

    static constexpr const char* description =
        "Adjust the proportion of the second exposure time in the total exposure time of the HDR "
        "exposure mode. The value of this parameter is usually smaller than the value of "
        "\"HdrExposureTimeProportion1\" but greater than "
        "(1-\"HdrExposureTimeProportion1\"-\"HdrExposureTimeProportion2\").";

    static constexpr Parameter::Type type = Parameter::Type::_Float;

    static constexpr const char* unit = "%";
};

class HdrFirstThreshold
{
public:
    static constexpr const char* name = "HdrFirstThreshold";

    static constexpr const char* description =
        "Set the maximum reachable grayscale value of the first exposure time of the HDR exposure "
        "mode. This is a percentage of the largest grayscale value 255.";

    static constexpr Parameter::Type type = Parameter::Type::_Float;

    static constexpr const char* unit = "%";
};

class HdrSecondThreshold
{
public:
    static constexpr const char* name = "HdrSecondThreshold";

    static constexpr const char* description =
        "Set the maximum reachable grayscale value of the second exposure time of the HDR exposure "
        "mode. This is a percentage of the largest grayscale value 255.";

    static constexpr Parameter::Type type = Parameter::Type::_Float;

    static constexpr const char* unit = "%";
};

class AnalogGain
{
public:
    static constexpr const char* name = "AnalogGain";

    static constexpr const char* description =
        "Increasing this parameter can enhance the brightness of the raw image. However, noise "
        "will also be introduced.\nIf the laser lines are dark, but a high \"MaxScanRate\" is "
        "needed, you can increase this parameter and decrease \"ExposureTime\".\nIf "
        "\"ExposureTime\" has reached its maximum value, but the laser lines are still too "
        "dark, you can increase this parameter.\nIf this parameter has reached its maximum value, "
        "but the laser lines are still too dark, increase \"DigitalGain\" or "
        "\"LaserPower\".\nNote: A large value of this parameter weakens the effect of HDR.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Gain_1,
        Gain_2,
        Gain_3,
        Gain_4,
        Gain_5, ///< LNX-8300, LNX-8080, LNX-75300, and LNX-7580 only.
    };
};

class [[deprecated(
    "Use class AnalogGain for LNX-8030 profilers with firmware version >= "
    "V2.3.0.")]] AnalogGainFor8030;
class AnalogGainFor8030
{
public:
    static constexpr const char* name = "AnalogGainFor8030";

    static constexpr const char* description =
        "Increasing this parameter can enhance the brightness of the raw image. However, noise "
        "will also be introduced.\nIf the laser lines are dark, but a high \"MaxScanRate\" is "
        "needed, you can increase this parameter and decrease \"ExposureTime\".\nIf "
        "\"ExposureTime\" has reached its maximum value, but the laser lines are still too "
        "dark, you can increase this parameter.\nIf this parameter has reached its maximum value, "
        "but the laser lines are still too dark, increase \"DigitalGain\" or "
        "\"LaserPower\".\nNote: A large value of this parameter weakens the effect of HDR.\n\n* "
        "Note: this parameter is deprecated since V2.3.0. If your profiler has firmware version >= "
        "V2.3.0, please use \"AnalogGain\" instead.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Gain_1_0,
        Gain_1_3,
        Gain_2_0,
        Gain_3_0,
    };
};

class DigitalGain
{
public:
    static constexpr const char* name = "DigitalGain";

    static constexpr const char* description =
        "Increasing this parameter can enhance the brightness of the raw image. However, a "
        "relatively large amount of noise will also be introduced.\nIf \"AnalogGain\" has reached "
        "its maximum value, but the laser lines are still too dark, you can increase \"Digital "
        "Gain\". \n\nNote: \"Digital Gain\" affects the minimum value of \"Min Grayscale "
        "Value\".";

    static constexpr Parameter::Type type = Parameter::Type::_Int;
};

class LaserPower
{
public:
    static constexpr const char* name = "LaserPower";

    static constexpr const char* description =
        "Set the power of the emitted laser, which affects the brightness of the laser lines in "
        "the raw image.\nIf the object is reflective or light-colored, you can decrease this "
        "parameter to reduce the brightness of the laser lines. If the object is unreflective or "
        "dark-colored, you can increase this parameter to enhance the brightness of the laser "
        "lines.\nNote: Even at the same power level, the brightness of the laser emitted by each "
        "device differs. Please adjust this parameter based on the actual condition of each "
        "device.";

    static constexpr Parameter::Type type = Parameter::Type::_Int;
};

} // namespace brightness_settings

namespace roi {

class ZDirectionRoi
{
public:
    static constexpr const char* name = "ZDirectionRoi";

    static constexpr const char* description =
        "A Z-direction ROI retains only the middle part of the raw image and trims the top and "
        "bottom parts off.\nSetting a Z-direction ROI can reduce the amount of data to be "
        "processed and enhance the \"MaxScanRate\".\nThe options are the ratio of the height of "
        "the trimmed image to the height of the untrimmed image.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        ImageHeight_1_1,
        ImageHeight_1_2,
        ImageHeight_1_4,
        ImageHeight_1_8,
        ImageHeight_1_16,
    };
};

class ROI
{
public:
    static constexpr const char* name = "ROI";

    static constexpr const char* description =
        "Setting an ROI in the XZ plane can reduce the amount of data to be processed, and enhance "
        "the data transmission speed and max scan rate.";

    static constexpr Parameter::Type type = Parameter::Type::_ProfileRoi;
};
} // namespace roi

} // namespace eye

} // namespace mmind
