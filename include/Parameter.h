#pragma once
#include <string>
#include <memory>
#include <map>
#include <vector>
#include "api_global.h"
#include "ErrorStatus.h"
#include "CommonTypes.h"

namespace Json {
class Value;
}
namespace mmind {

class ZmqClientImpl;

namespace eye {

/**
 * @brief Represents a parameter of the device.
 */
class MMIND_API_EXPORT Parameter
{
    friend class ParameterFactory;

public:
    /**
     * @brief Describes the device parameter data types.
     */
    enum Type {
        _Int,        ///< Integer type.
        _Float,      ///< Double type.
        _Bool,       ///< Boolean type.
        _Enum,       ///< Enumeration type.
        _Roi,        ///< %ROI type. See @ref ROI for details.
        _Range,      ///< %Range type. See @ref Range for details.
        _FloatArray, ///< Vector of double types.
        _RoiArray,   ///< Vector of ROI types.
        _ProfileRoi  ///< Profile ROI type. See @ref ProfileROI for details.
    };

    virtual ~Parameter();

    /**
     * @brief Returns the name of the device parameter.
     */
    std::string name() const;

    /**
     * @brief Returns the data type of the device parameter.
     */
    Type type() const;

    /**
     * @brief Returns the description of the device parameter.
     */
    std::string description() const;

    /**
     * @brief Returns a Boolean value that indicates the write permission of the device parameter.
     */
    bool isWritable() const;

    /**
     * @brief Returns a Boolean value that indicates the read permission of the device parameter.
     */
    bool isReadable() const;

protected:
    std::shared_ptr<class ParameterImpl> _impl;
    Parameter(const std::string& name, const std::shared_ptr<ZmqClientImpl>& client,
              const std::shared_ptr<Json::Value>& parameterInfo, bool needUpdateMaxAndMin = false,
              bool isVirtual = false);
};

/**
 * @brief Represents an _Int-type device parameter.
 */
class MMIND_API_EXPORT IntParameter : public Parameter
{
public:
    /**
     * @brief Gets the current value of the device parameter.
     */
    ErrorStatus getValue(int& value) const;

    /**
     * @brief Sets the value of the device parameter.
     */
    ErrorStatus setValue(int value);

    /**
     * @brief Gets the minimum settable value of the device parameter.
     */
    ErrorStatus getMin(int& min) const;

    /**
     * @brief Gets the maximum settable value of the device parameter.
     */
    ErrorStatus getMax(int& max) const;

    /**
     * @brief Gets the adjustment step size of the device parameter.
     */
    ErrorStatus getStep(int& step) const;

    /**
     * @brief Gets the unit of the device parameter.
     */
    ErrorStatus getUnit(std::string& unit) const;

protected:
    using Parameter::Parameter;
};

/**
 * @brief Represents a _Float-type device parameter.
 */
class MMIND_API_EXPORT FloatParameter : public Parameter
{
public:
    /**
     * @brief Gets the current value of the device parameter.
     */
    ErrorStatus getValue(double& value) const;

    /**
     * @brief Sets the value of the device parameter.
     */
    ErrorStatus setValue(double value);

    /**
     * @brief Gets the minimum settable value of the device parameter.
     */
    ErrorStatus getMin(double& min) const;

    /**
     * @brief Gets the maximum settable value of the device parameter.
     */
    ErrorStatus getMax(double& max) const;

    /**
     * @brief Gets the adjustment step size of the device parameter.
     */
    ErrorStatus getStep(double& step) const;

    /**
     * @brief Gets the unit of the device parameter.
     */
    ErrorStatus getUnit(std::string& unit) const;

protected:
    using Parameter::Parameter;
};

/**
 * @brief Represents an _Enum-type device parameter.
 */
class MMIND_API_EXPORT EnumParameter : public Parameter
{
public:
    /**
     * @brief Gets the current value of the device parameter in the form of the integer value.
     */
    ErrorStatus getValue(int& value) const;

    /**
     * @brief Sets the value of the device parameter by inputting the integer value of an
     * enumerator.
     */
    ErrorStatus setValue(int value);

    /**
     * @brief Gets the current value of the device parameter in the form of a string.
     */
    ErrorStatus getValue(std::string& valueStr) const;

    /**
     * @brief Sets the value of the device parameter by inputting a string.
     */
    ErrorStatus setValue(const std::string& value);

    /**
     * @brief Gets the list of available values of the device parameter in the form of a map.
     */
    ErrorStatus getValues(std::map<std::string, int>& valueList) const;

protected:
    using Parameter::Parameter;
};

/**
 * @brief Represents a _Bool-type device parameter.
 */
class MMIND_API_EXPORT BoolParameter : public Parameter
{
public:
    /**
     * @brief Gets the current value of the device parameter.
     */
    ErrorStatus getValue(bool& value) const;

    /**
     * @brief Sets the value of the device parameter.
     */
    ErrorStatus setValue(bool value);

protected:
    using Parameter::Parameter;
};

/**
 * @brief Represents an _Roi-type device parameter.
 */
class MMIND_API_EXPORT RoiParameter : public Parameter
{
public:
    /**
     * @brief Gets the current value of the device parameter.
     */
    ErrorStatus getValue(ROI& value) const;

    /**
     * @brief Sets the value of the device parameter.
     */
    ErrorStatus setValue(ROI value);

    /**
     * @brief Gets the maximum settable value of the device parameter.
     */
    ErrorStatus getMaxRoiSize(Size& maxSize) const;

protected:
    using Parameter::Parameter;
};

/**
 * @brief Represents an _ProfileRoi-type device parameter.
 */
class MMIND_API_EXPORT ProfileRoiParameter : public Parameter
{
public:
    /**
     * @brief Gets the current value of the device parameter.
     */
    ErrorStatus getValue(ProfileROI& value) const;

    /**
     * @brief Sets the value of the device parameter.
     */
    ErrorStatus setValue(ProfileROI value);

    /**
     * @brief Gets the maximum settable value of the device parameter.
     */
    ErrorStatus getMaxRoiSize(SizeF& maxSize) const;

    /**
     * @brief Gets the minimum settable value of the device parameter.
     */
    ErrorStatus getMinRoiSize(SizeF& minSize) const;

protected:
    using Parameter::Parameter;
};

/**
 * @brief Represents an _RoiArray-type device parameter.
 */
class MMIND_API_EXPORT RoiArrayParameter : public Parameter
{
public:
    /**
     * @brief Gets the current value of the device parameter.
     */
    ErrorStatus getValue(std::vector<ROI>& value) const;

protected:
    using Parameter::Parameter;
};

/**
 * @brief Represents a _Range-type device parameter.
 */
class MMIND_API_EXPORT RangeParameter : public Parameter
{
public:
    /**
     * @brief Gets the current value of the device parameter.
     */
    ErrorStatus getValue(Range<int>& value) const;

    /**
     * @brief Sets the value of the device parameter.
     */
    ErrorStatus setValue(Range<int> value);

    /**
     * @brief Gets the minimum settable value of the device parameter.
     */
    ErrorStatus getMin(int& min) const;

    /**
     * @brief Gets the maximum settable value of the device parameter.
     */
    ErrorStatus getMax(int& max) const;

    /**
     * @brief Gets the adjustment step size of the device parameter.
     */
    ErrorStatus getStep(int& step) const;

    /**
     * @brief Gets the unit of the device parameter.
     */
    ErrorStatus getUnit(std::string& unit) const;

protected:
    using Parameter::Parameter;
};

/**
 * @brief Represents a _FloatArray-type device parameter.
 */
class MMIND_API_EXPORT FloatArrayParameter : public Parameter
{
public:
    /**
     * @brief Gets the current value of the device parameter.
     */
    ErrorStatus getValue(std::vector<double>& value) const;

    /**
     * @brief Sets the value of the device parameter.
     */
    ErrorStatus setValue(const std::vector<double>& value);

    /**
     * @brief Gets the minimum settable value of the device parameter.
     */
    ErrorStatus getMin(double& min) const;

    /**
     * @brief Gets the maximum settable value of the device parameter.
     */
    ErrorStatus getMax(double& max) const;

    /**
     * @brief Gets the maximum settable size of the vector.
     */
    ErrorStatus getMaxArraySize(int& maxSize) const;

    /**
     * @brief Gets the adjustment step size of the device parameter.
     */
    ErrorStatus getStep(double& step) const;

    /**
     * @brief Gets the unit of the device parameter.
     */
    ErrorStatus getUnit(std::string& unit) const;

protected:
    using Parameter::Parameter;
};

} // namespace eye
} // namespace mmind
