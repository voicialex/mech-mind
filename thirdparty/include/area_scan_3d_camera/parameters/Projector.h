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

namespace projector_setting {

class PowerLevel
{
public:
    static constexpr const char* name = "ProjectorPowerLevel";

    static constexpr const char* description = "Set the brightness level of the projector.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        High,   ///< High level is often used for scanning dark objects.
        Normal, ///< Normal level is mostly used.
        Low,    ///< Low level is used for scanning reflective objects.
    };
};

class FringeCodingMode
{
public:
    static constexpr const char* name = "ProjectorFringeCodingMode";

    static constexpr const char* description =
        "Selects the pattern of the structured light to be projected.\n\nNote:\n* When "
        "\"Translucent\" is selected, the following tool and parameters are unavailable:\n** "
        "\"AntiFlickerMode\" in the \"mmind::eye::projector_setting\" namespace\n** "
        "\"EdgePreservation\", \"EnableDistortionCorrection\", and \"DistortionCorrection\" in the "
        "\"mmind::eye::pointcloud_processing_setting\" namespace\n* When \"Translucent\" is "
        "selected, you can fill in the missing points in the point cloud by adjusting "
        "\"GapFilling\" in the \"mmind::eye::projector_setting\" namespace.\n* When \"Reflective\" "
        "is selected, the following tool and parameters are unavailable:\n** \"AntiFlickerMode\" "
        "in the \"mmind::eye::projector_setting\" namespace\n** \"EnableDistortionCorrection\", "
        "\"DistortionCorrection\" and \"FringeMinThreshold\" in the "
        "\"mmind::eye::pointcloud_processing_setting\" namespace";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Fast, ///< Suitable for opaque objects, provides fast acquisition speed, but the depth data
              ///< quality is lower.
        Accurate,    ///< Suitable for opaque objects, provides high-quality depth data, but the
                     ///< acquisition speed is slower.
        Translucent, ///< PRO S and PRO M only. Suitable for translucent objects, provides
                     ///< high-quality depth data, but the acquisition speed is slower.
        Reflective,  ///< PRO S and PRO M only. Suitable for reflective objects, provides
                     ///< high-quality depth data, but the acquisition speed is slower.
    };
};

class AntiFlickerMode
{
public:
    static constexpr const char* name = "AntiFlickerMode";

    static constexpr const char* description =
        "Flicker refers to the rapid and periodical change in the intensity of artificial light. "
        "This phenomenon can cause fluctuations in the depth data. Such fluctuation can be reduced "
        "by adjusting the projection frequency of the structured light.\n\nNote: "
        "\"AntiFlickerMode\" is unavailable when \"FringeCodingMode\" is set to \"Translucent\" or "
        "\"Reflective\".";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Off,    ///< No processing for anti-flicker.
        AC50Hz, ///< The AC frequency is 50Hz in most countries.
        AC60Hz, ///< The AC frequency in the U.S. and some Asian countries is 60Hz.
    };
};

class ProcessingMode
{
public:
    static constexpr const char* name = "ProjectorProcessingMode";

    static constexpr const char* description =
        "Select the data processing mode for the \"Reflective\" fringe coding mode.\n\n* Faster: "
        "provides faster processing speed, but the depth data might have missing points. Suitable "
        "for scenarios with relatively simple reflective conditions.\n* More Complete: provides "
        "more complete depth data, but the processing speed is slower. Suitable for scenarios with "
        "complex reflective conditions, such as a bin whose walls often cause interreflection.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Faster, ///< Provides faster processing speed, but the depth data might have missing points.
        ///< Suitable for scenarios with relatively simple reflective conditions.
        MoreComplete, ///< Provides more complete depth data, but the processing speed is slower.
        ///< Suitable for scenarios with complex reflective conditions, such as a bin
        ///< whose walls often cause interreflection.
    };
};

} // namespace projector_setting

} // namespace eye

} // namespace mmind
