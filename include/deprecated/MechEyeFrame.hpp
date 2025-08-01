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
#include <cstdint>
#include <memory>
#include <vector>
#include <stdexcept>
#include "api_global.h"
#include "MechEyeDataType.h"

namespace mmind {
namespace api {

template <typename ElementType>
class Frame;

/**
 * @brief Element in DepthMap.
 */
struct ElementDepth
{
    float d{0}; ///< Depth channel, unit: mm.
};

/**
 * @brief Element in ColorMap.
 */
struct ElementColor
{
    uint8_t b{0}; ///< Blue channel.
    uint8_t g{0}; ///< Green channel.
    uint8_t r{0}; ///< Red channel.
};

/**
 * @brief Element in PointXYZMap.
 */
struct ElementPointXYZ
{
    float x{0}; ///< X channel, unit: mm.
    float y{0}; ///< Y channel, unit: mm.
    float z{0}; ///< Z channel, unit: mm.
};

/**
 * @brief Element in PointXYZBGRMap.
 */
struct ElementPointXYZBGR
{
    float x{0};   ///< X channel, unit: mm.
    float y{0};   ///< Y channel, unit: mm.
    float z{0};   ///< Z channel, unit: mm.
    uint8_t b{0}; ///< Blue channel.
    uint8_t g{0}; ///< Green channel.
    uint8_t r{0}; ///< Red channel.
};

typedef Frame<ElementColor> ColorMap;

typedef Frame<ElementDepth> DepthMap;

typedef Frame<ElementPointXYZ> PointXYZMap;

typedef Frame<ElementPointXYZBGR> PointXYZBGRMap;

/**
 * @brief Definition of data structure in device capturing image
 */
template <typename ElementType>
class Frame
{
public:
    /**
     * Constructor
     */
    Frame() : _width(0), _height(0), _pData(nullptr) {}
    /**
     * Destructor
     */
    ~Frame() {}

    /**
     * Returns the width of the Frame
     */
    inline uint32_t width() const { return _width; }

    /**
     * Returns the height of the Frame
     */
    inline uint32_t height() const { return _height; }

    /**
     * Returns true if the Frame has no elements.
     */
    inline bool empty() const { return !_pData; }

    /**
     * Returns the pointer to the element data.
     */
    inline const ElementType* data() const { return _pData.get(); }

    /**
     * Returns the pointer to the element data.
     */
    inline ElementType* data()
    {
        return const_cast<ElementType*>(static_cast<const Frame<ElementType>&>(*this).data());
    }

    /**
     * Returns a const element reference to the specified index in the Frame using the operator [].
     * @param n Index along the one dimension. It will throw an exception if the input n
     * is greater than @ref width * @ref height.
     */
    inline const ElementType& operator[](std::size_t n) const
    {
        if (n >= _height * _width || !_pData)
            throw std::out_of_range("invalid subscript");
        ElementType* data = _pData.get();
        return data[n];
    }

    /**
     * Returns a element reference to the specified index in the Frame using the operator [].
     * @param n Index along the one dimension. It will throw an exception if the input n
     * is greater than @ref width * @ref height.
     */
    inline ElementType& operator[](std::size_t n)
    {
        return const_cast<ElementType&>(static_cast<const Frame<ElementType>&>(*this)[n]);
    }

    /**
     * Returns a const element reference to the specified row and col index in the Frame.
     * @param row Index along the dimension height. It will throw an exception if the input row
     * is greater than @ref width.
     * @param col Index along the dimension width. It will throw an exception if the input col
     * is greater than @ref height.
     */
    const ElementType& at(uint32_t row, uint32_t col) const
    {
        if (row >= _height || col >= _width || !_pData)
            throw std::out_of_range("invalid subscript");
        ElementType* data = _pData.get();
        return data[row * _width + col];
    }

    /**
     * Returns an element reference to the specified row and col index in the Frame.
     * @param row Index along the dimension height. It will throw an exception if the input row
     * is greater than @ref width.
     * @param col Index along the dimension width. It will throw an exception if the input col
     * is greater than @ref height.
     */
    ElementType& at(uint32_t row, uint32_t col)
    {
        return const_cast<ElementType&>(static_cast<const Frame<ElementType>&>(*this).at(row, col));
    }

    /**
     * Change the Frame size to a new one. It will destroy the existing data and reallocate memory
     * according to the new size.
     * @param width New number of the Frame width.
     * @param height New number of the Frame height.
     */
    void resize(uint32_t width, uint32_t height)
    {
        if (_width == width && _height == height)
            return;

        _width = width;
        _height = height;
        _pData.reset(new ElementType[_width * _height], [](ElementType* p) { delete[] p; });
    }

    /**
     * Deallocated the Frame data.
     */
    void release()
    {
        _pData.reset();
        _width = 0;
        _height = 0;
    }

private:
    uint32_t _width;
    uint32_t _height;
    std::shared_ptr<ElementType> _pData;
};

class LineBatch
{
public:
    LineBatch() : _lineCount(0), _columnCount(0) {}
    ~LineBatch() {}

    const Frame<float>& depth() const { return _depth; }
    Frame<float>& depth() { return _depth; }

    const Frame<unsigned char>& intensity() const { return _intensity; }
    Frame<unsigned char>& intensity() { return _intensity; }

    const Frame<unsigned int>& encoder() const { return _encoder; }
    Frame<unsigned int>& encoder() { return _encoder; }

    const Frame<long long>& frameId() const { return _frameId; }
    Frame<long long>& frameId() { return _frameId; }

    void setLineCount(uint32_t lineCount) { _lineCount = lineCount; }
    uint32_t lineCount() const { return _lineCount; }
    uint32_t columnCount() const { return _columnCount; }

    float depthAt(uint32_t row, uint32_t col) const { return _depth.at(row, col); }
    unsigned char intensityAt(uint32_t row, uint32_t col) const { return _intensity.at(row, col); }
    unsigned int encoderAt(uint32_t row, uint32_t col) const { return _encoder.at(row, col); }
    long long frameIdAt(uint32_t row, uint32_t col) const { return _frameId.at(row, col); }

    void resize(uint32_t width, uint32_t height)
    {
        _columnCount = width;
        _lineCount = height;
        _depth.resize(_columnCount, _lineCount);
        _intensity.resize(_columnCount, _lineCount);
        _encoder.resize(1, _lineCount);
        _frameId.resize(1, _lineCount);
    }

private:
    Frame<float> _depth;
    Frame<unsigned char> _intensity;
    Frame<unsigned int> _encoder;
    Frame<long long> _frameId;
    uint32_t _lineCount;
    uint32_t _columnCount;
};

} // namespace api
} // namespace mmind
