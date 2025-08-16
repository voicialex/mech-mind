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

namespace laser_setting {

class PowerLevel
{
public:
    static constexpr const char* name = "LaserPowerLevel";

    static constexpr const char* description =
        "Set Laser's power level."
        "High power is often used for scanning dark objects. Low power is often used for scanning "
        "reflective objects.";

    static constexpr Parameter::Type type = Parameter::Type::_Int;

    static constexpr int step() { return 10; }

    static constexpr Range<int> range() { return {50, 100}; }

    static constexpr const char* unit = "%";
};

class FringeCodingMode
{
public:
    static constexpr const char* name = "LaserFringeCodingMode";

    static constexpr const char* description =
        "Selects the pattern of the structured light to be projected.\n\nNote:\n* When "
        "\"Reflective\" is selected, the following tool and parameters are "
        "unavailable:\n\"FramePartitionCount\" in the \"mmind::eye::laser_setting\" namespace\n** "
        "\"FringeMinThreshold\" in the \"mmind::eye::pointcloud_processing_setting\" namespace.";

    static constexpr Parameter::Type type = Parameter::Type::_Enum;

    enum struct Value {
        Fast,     ///< Suitable for non-reflective objects, provides fast acquisition speed, but the
                  ///< depth data quality is lower.
        Accurate, ///< Suitable for non-reflective objects, provides high-quality depth data, but
                  ///< the acquisition speed is slower.
        Reflective, ///< Suitable for reflective objects, provides high-quality depth data, but the
                    ///< acquisition speed is slower.
    };
};

class ProcessingMode
{
public:
    static constexpr const char* name = "LaserProcessingMode";

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

class FrameRange
{
public:
    static constexpr const char* name = "LaserFrameRange";

    static constexpr const char* description = "Modify the Laser's scan range.";

    static constexpr Parameter::Type type = Parameter::Type::_Range;

    static constexpr Range<int> range() { return {0, 100}; }
};

class FramePartitionCount
{
public:
    static constexpr const char* name = "LaserFramePartitionCount";

    static constexpr const char* description =
        "Set Laser's scan partition count."
        "If the value is more than 1, the scan from start to end will be partitioned to "
        "multiple "
        "parts. It is recommended to use mutiple partition for extremely dark "
        "objects.\n\nNote: "
        "\"FramePartitionCount\" is unavailable when \"FringeCodingMode\" category is set to "
        "\"Reflective\".";

    static constexpr Parameter::Type type = Parameter::Type::_Int;

    static constexpr Range<int> range() { return {1, 4}; }
};
} // namespace laser_setting

} // namespace eye

} // namespace mmind
