#pragma once
#include <memory>
#include <vector>
#include "api_global.h"
#include "ErrorStatus.h"
#include "Parameter.h"

namespace mmind {
namespace eye {
class VirtualUserSetImpl;

class MMIND_API_EXPORT VirtualUserSet
{
public:
    /**
     * @brief Destructor.
     */
    virtual ~VirtualUserSet();

    /**
     * @brief Gets the name of the parameter group used when the virtual device was saved.
     * @param [out] userSetName The name of the parameter group.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     */
    ErrorStatus getName(std::string& userSetName) const;

    /**
     * @brief Gets the names of all available parameters of the virtual device.
     * @param [out] parameterArrayNames An array storing the parameter names.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     */
    ErrorStatus getAvailableParameterNames(std::vector<std::string>& parameterArrayNames) const;

    /**
     * @brief Returns the pointer to the elements in the vector storing all available parameters.
     */
    std::vector<Parameter*> getAvailableParameters() const;

    /**
     * @brief Gets the pointer to the elements of a specific parameter.
     */
    Parameter* getParameter(const std::string& parameterName) const;

    /**
     * @brief Gets the pointer to the elements of a specific parameter.
     */
    Parameter* operator[](const std::string& parameterName) const;

    /**
     * @brief Gets the current value of an _Int-type parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getIntValue(const std::string& parameterName, int& value) const;

    /**
     * @brief Gets the current value of a _Float-type parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getFloatValue(const std::string& parameterName, double& value) const;

    /**
     * @brief Gets the current value of a _Bool-type parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getBoolValue(const std::string& parameterName, bool& value) const;

    /**
     * @brief Gets the current value of an _Enum-type parameter in the form of the integer
     * value. See @ref Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getEnumValue(const std::string& parameterName, int& value) const;

    /**
     * @brief Gets the current value of an _Enum-type parameter in the form of a string. See
     * @ref Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getEnumValue(const std::string& parameterName, std::string& valueStr) const;

    /**
     * @brief Gets the current value of an _Roi-type parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the input
     *  parameter name.\n
     */
    ErrorStatus getRoiValue(const std::string& parameterName, ROI& value) const;

    /**
     * @brief Gets the current value of an _ProfileRoi-type parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the input
     *  parameter name.\n
     */
    ErrorStatus getProfileRoiValue(const std::string& parameterName, ProfileROI& value) const;

    /**
     * @brief Gets the current value of a _FloatArray-type parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getFloatArrayValue(const std::string& parameterName,
                                   std::vector<double>& value) const;

    /**
     * @brief Gets the current value of a _Range-type parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getRangeValue(const std::string& parameterName, Range<int>& value) const;

private:
    std::shared_ptr<class VirtualUserSetImpl> _impl;
    explicit VirtualUserSet();
    friend class VirtualProfilerImpl;
};

} // namespace eye
} // namespace mmind
