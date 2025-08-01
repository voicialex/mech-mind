#pragma once
#include <cstring>
#include <memory>
#include <vector>
#include <stdexcept>

namespace mmind {
namespace eye {

/**
 * @brief Represents the data struct of the profile data.
 */
template <typename ElementData>
class BatchArray
{
public:
    /**
     * @brief Describes a constructor.
     */
    BatchArray(size_t width) : _width(width) {}
    /**
     * @brief Describes a destructor.
     */
    ~BatchArray() = default;

    /**
     * @brief Returns the width of the BatchArray object.
     */
    size_t width() const { return _width; }

    /**
     * @brief Returns the height of the BatchArray object.
     */
    size_t height() const { return _height; }

    /**
     * @brief Sets the height of the BatchArray object.
     */
    void setHeight(size_t height)
    {
        if (height > _capacity) {
            reserve(height);
        }
        _height = height;
    }

    /**
     * @brief Returns the size of the storage space currently allocated for the BatchArray object,
     * expressed in number of lines.
     */
    size_t capacity() const { return _capacity; }

    /**
     * @brief Returns true if the BatchArray object has no elements.
     */
    bool isEmpty() const { return _height == 0; }

    /**
     * @brief Requests for enough capacity of the BatchArray object to contain
     * the number of lines corresponding to the height.
     */
    void reserve(size_t height)
    {
        if (_capacity >= height)
            return;

        std::shared_ptr<ElementData> pNewData(new ElementData[_width * height],
                                              [](ElementData* p) { delete[] p; });
        if (_pData) {
            memcpy(pNewData.get(), _pData.get(), _width * _height * sizeof(ElementData));
        }
        _capacity = height;
        _pData = std::move(pNewData);
    }

    /**
     * @brief Appends the data variable onto the end of this BatchArray object.
     */
    bool append(const BatchArray& data)
    {
        if (_width != data.width())
            return false;

        if (_capacity - _height < data.height()) {
            reserve(data.height() + _height);
        }
        memcpy(_pData.get() + _height * _width, data.data(),
               data.height() * data.width() * sizeof(ElementData));
        _height += data.height();
        return true;
    }

    /**
     * @brief Returns the pointer to the element data.
     */
    const ElementData* data() const { return _pData.get(); }

    /**
     * @brief Returns the pointer to the element data.
     */
    ElementData* data() { return _pData.get(); }

    /**
     * @brief Returns a const element reference to the specified index in the BatchArray object
     * using the operator [].
     * @param n Index of an element. An exception is thrown if the input n
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
     * @brief Returns an element reference to the specified index in the BatchArray object using the
     * operator [].
     * @param n Index of an element. An exception is thrown if the input n
     * is greater than @ref width * @ref height.
     */
    ElementData& operator[](std::size_t n)
    {
        return const_cast<ElementData&>(static_cast<const BatchArray<ElementData>&>(*this)[n]);
    }

    /**
     * @brief Returns a const element reference to the specified row and column index in the
     * BatchArray object.
     * @param row Index along the height dimension. An exception is thrown if the input row
     * is greater than @ref width.
     * @param col Index along the width dimension. An exception is thrown if the input col
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
     * @brief Returns an element reference to the specified row and column index in the BatchArray
     * object.
     * @param row Index along the height dimension. An exception is thrown if the input row
     * is greater than @ref width.
     * @param col Index along the width dimension. An exception is thrown if the input col
     * is greater than @ref height.
     */
    ElementData& at(size_t row, size_t col)
    {
        return const_cast<ElementData&>(
            static_cast<const BatchArray<ElementData>&>(*this).at(row, col));
    }

    /**
     * @brief Creates a deep copy of the BatchArray object.
     */
    BatchArray<ElementData> clone() const
    {
        BatchArray<ElementData> copy(_width);
        copy.reserve(_height);
        memcpy(copy.data(), data(), _height * _width * sizeof(ElementData));
        copy.setHeight(_height);
        return copy;
    }

    /**
     * @brief Clears the data of the BatchArray object.
     */
    void clear()
    {
        memset(_pData.get(), 0, _height * _width * sizeof(ElementData));
        _height = 0;
    }

private:
    size_t _width{0};
    size_t _height{0};
    size_t _capacity{0};
    std::shared_ptr<ElementData> _pData;
};
} // namespace eye
} // namespace mmind
