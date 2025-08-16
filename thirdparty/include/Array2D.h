#pragma once
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <stdexcept>

namespace mmind {

namespace eye {

/**
 * @brief Represents a 2D container of data.
 */
template <typename ElementData>
class Array2D
{
public:
    /**
     * @brief Constructor.
     */
    Array2D() : _width(0), _height(0), _pData(nullptr) {}
    /**
     * @brief Describes a destructor.
     */
    ~Array2D() {}

    /**
     * @brief Returns the width of the Array2D object.
     */
    size_t width() const { return _width; }

    /**
     * @brief Returns the height of the Array2D object.
     */
    size_t height() const { return _height; }

    /**
     * @brief Returns true if the Array2D object has no elements.
     */
    bool isEmpty() const { return !_pData; }

    /**
     * @brief Returns the pointer to an element in the Array2D object. The returned pointer will be
     * invalidated after
     * @ref resize or @ref release is called.
     */
    const ElementData* data() const { return _pData.get(); }

    /**
     * @brief Returns the pointer to an element in the Array2D object. The returned pointer will be
     * invalidated after
     * @ref resize or @ref release is called.
     */
    ElementData* data()
    {
        return const_cast<ElementData*>(static_cast<const Array2D<ElementData>&>(*this).data());
    }

    /**
     * @brief Returns a reference to the constant element with the specified index in the Array2D
     * object using the operator [].
     * @param [in] n The index of an element. An exception is thrown if the input n
     * is greater than @ref width * @ref height.
     */
    const ElementData& operator[](std::size_t n) const
    {
        if (n >= _height * _width || !_pData)
            throw std::out_of_range("invalid subscript");
        ElementData* data = _pData.get();
        return data[n];
    }

    /**
     * @brief Returns a reference to the constant element with the specified index in the Array2D
     * object using the operator [].
     * @param [in] n The index of an element. An exception is thrown if the input n is greater than
     * @ref width * @ref height.
     */
    ElementData& operator[](std::size_t n)
    {
        return const_cast<ElementData&>(static_cast<const Array2D<ElementData>&>(*this)[n]);
    }

    /**
     * @brief Returns a reference to the constant element at the specified row and column in the
     * Array2D object.
     * @param [in] row The index along the height dimension. An exception is thrown if the input row
     * is greater than @ref width.
     * @param [in] col The index along the width dimension. An exception is thrown if the input col
     * is greater than @ref height.
     */
    const ElementData& at(uint32_t row, uint32_t col) const
    {
        if (row >= _height || col >= _width || !_pData)
            throw std::out_of_range("invalid subscript");
        ElementData* data = _pData.get();
        return data[row * _width + col];
    }

    /**
     * @brief Returns a reference to the element at the specified row and column in the Array2D
     * object.
     * @param row The index along the height dimension. An exception is thrown if the input row
     * is greater than @ref width.
     * @param col The index along the width dimension. An exception is thrown if the input col
     * is greater than @ref height.
     */
    ElementData& at(uint32_t row, uint32_t col)
    {
        return const_cast<ElementData&>(
            static_cast<const Array2D<ElementData>&>(*this).at(row, col));
    }

    /**
     * @brief Creates a deep copy of the Array2D object.
     */
    Array2D<ElementData> clone() const
    {
        Array2D<ElementData> copy;
        copy.resize(_width, _height);
        memcpy(copy.data(), data(), _height * _width * sizeof(ElementData));
        return copy;
    }
    /**
     * @brief Changes the size of the Array2D object. It destroys the existing data and reallocates
     * memory according to the new size, if the new size is different from the old size.
     * @param [in] width The new width of the Array2D object.
     * @param [in] height The new height of the Array2D object.
     */
    void resize(size_t width, size_t height)
    {
        if (width == 0 || height == 0)
            return release();

        if (_width == width && _height == height)
            return;

        _width = width;
        _height = height;
        _pData.reset(new ElementData[_width * _height], [](ElementData* p) { delete[] p; });
    }

    /**
     * @brief Deallocates the data in the Array2D object.
     */
    void release()
    {
        _pData.reset();
        _width = 0;
        _height = 0;
    }

private:
    size_t _width;
    size_t _height;
    std::shared_ptr<ElementData> _pData;
};

} // namespace eye
} // namespace mmind
