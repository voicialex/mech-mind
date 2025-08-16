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
#include <memory>
#include "api_global.h"
#include "CameraProperties.h"
#include "Array2D.h"

namespace mmind {

namespace eye {

/**
 * @brief Represents a pixel in @ref GrayScale2DImage with grayscale channel information.
 */
struct Gray
{
    uint8_t gray{0}; ///< Grayscale channel.
};

/**
 * @brief Represents a pixel in @ref Color2DImage with blue, green, and red channel information.
 */
struct ColorBGR
{
    uint8_t b{0}; ///< Blue channel.
    uint8_t g{0}; ///< Green channel.
    uint8_t r{0}; ///< Red channel.
};

using GrayScale2DImage = Array2D<Gray>;
using Color2DImage = Array2D<ColorBGR>;

/**
 * @brief Represents the 2D capture result, which can be obtained by calling
 * @ref Camera.capture2D. The 2D data can be in the form of @ref Gray or @ref ColorBGR and is stored
 * in a 2D array, with each element in the array representing an image pixel.
 */
class MMIND_API_EXPORT Frame2D
{
public:
    /**
     * @brief Constructor.
     */
    Frame2D();

    /**
     * @brief Destructor.
     */
    ~Frame2D();

    /**
     * @brief Constructor.
     */
    Frame2D(const Frame2D& other) noexcept;

    /**
     * @brief Copy assignment.
     */
    Frame2D& operator=(const Frame2D& other) noexcept;

    /**
     * @brief Gets the image size of @ref Frame2D.
     */
    Size imageSize() const;

    /**
     * @brief Judges whether @ref Frame2D is empty.
     */
    bool isEmpty() const;

    /**
     * @brief Gets the color type of the 2D camera in the 3D camera.
     */
    ColorTypeOf2DCamera colorType() const;

    /**
     * @brief The ID of the @ref Frame2D frame.
     */
    uint64_t frameId() const;

    /**
     * @brief Clears the data in @ref Frame2D and releases the associated resources.
     */
    void clear();

    /**
     * @brief Gets image data of the @ref Frame2D with the @ref Gray pixel format. If
     * the 2D camera's color type (according to the return value of @ref Frame2D.colorType) is @ref
     * Frame2DColorType.Monochrome, this method gets the original data. If the color type is @ref
     * Frame2DColorType.Color, this method converts @ref ColorBGR values to @ref Gray values
     * according to the following equation: Gray = 0.299 * Red + 0.587 * Green + 0.114 * Blue.
     * @return See @ref GrayScale2DImage for details.
     */
    GrayScale2DImage getGrayScaleImage() const;

    /**
     * @brief Gets the image data of the @ref Frame2D with the @ref ColorBGR pixel format.
     * If the 2D camera's color type (according to the return value of @ref Frame2D.colorType) is
     * @ref Frame2DColorType.Color, this method gets the original data. If the color type is @ref
     * Frame2DColorType.Monochrome, this method converts the @ref Gray values to @ref ColorBGR
     * values according to the following equation: Blue = Gray, Green = Gray, Red = Gray.
     * @return See @ref Color2DImage for details.
     */
    Color2DImage getColorImage() const;

private:
    friend class CameraImpl;
    friend class Frame2DAnd3D;
    friend class InternalInterfaces;

    std::shared_ptr<class Frame2DImpl> _impl;
    Frame2D(std::shared_ptr<Frame2DImpl>& frameImpl);
};

} // namespace eye
} // namespace mmind
