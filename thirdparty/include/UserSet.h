#pragma once
#include <memory>
#include <vector>
#include "api_global.h"
#include "ErrorStatus.h"
#include "Parameter.h"

namespace mmind {
class ZmqClientImpl;
namespace eye {

class MMIND_API_EXPORT UserSet
{
public:
    /**
     * @brief Destructor.
     */
    virtual ~UserSet();

    /**
     * @brief Gets the name of the current device user set.
     * @param [out] userSetName The current user set name.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     */
    ErrorStatus getName(std::string& userSetName) const;

    /**
     * @brief Renames the current device user set.
     * @param [in] newName The new user set name.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_INPUT_ERROR The user set name entered is empty or too
     *  long.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     */
    ErrorStatus rename(const std::string& newName);

    /**
     * @brief Gets the names of all available device parameters in the current device user set.
     * @param [out] parameterArrayNames An array storing the parameter names.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     */
    ErrorStatus getAvailableParameterNames(std::vector<std::string>& parameterArrayNames) const;

    /**
     * @brief Resets all device parameters in the current device user set to default values.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     */
    ErrorStatus resetAllParametersToDefaultValues();

    /**
     * @brief Saves the values of all device parameters in the current device user set to the
     * device.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     */
    ErrorStatus saveAllParametersToDevice() const;

    /**
     * @brief Returns the pointers to the elements in the vector storing all available parameters.
     */
    std::vector<Parameter*> getAvailableParameters() const;

    /**
     * @brief Gets the pointer to the specified device parameter.
     */
    Parameter* getParameter(const std::string& parameterName) const;

    /**
     * @brief Gets the pointer to the specified device parameter.
     */
    Parameter* operator[](const std::string& parameterName) const;

    /**
     * @brief Gets the current value of an _Int-type device parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getIntValue(const std::string& parameterName, int& value) const;

    /**
     * @brief Sets the value of an _Int-type device parameter. See @ref Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found or read only,
     *  please check the input parameter name.\n
     *  @ref ErrorStatus.MMIND_STATUS_OUT_OF_RANGE_ERROR Invalid parameter input.\n
     */
    ErrorStatus setIntValue(const std::string& parameterName, int value);

    /**
     * @brief Gets the current value of a _Float-type device parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getFloatValue(const std::string& parameterName, double& value) const;

    /**
     * @brief Sets the value of a _Float-type device parameter. See @ref Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found or read only,
     *  please check the input parameter name.\n
     *  @ref ErrorStatus.MMIND_STATUS_OUT_OF_RANGE_ERROR Invalid parameter input.\n
     */
    ErrorStatus setFloatValue(const std::string& parameterName, double value);

    /**
     * @brief Gets the current value of a _Bool-type device parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getBoolValue(const std::string& parameterName, bool& value) const;

    /**
     * @brief Sets the value of a _Bool-type device parameter. See @ref Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found or read only,
     *  please check the input parameter name.\n
     */
    ErrorStatus setBoolValue(const std::string& parameterName, bool value);

    /**
     * @brief Gets the current value of an _Enum-type device parameter in the form of the integer
     * value. See @ref Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getEnumValue(const std::string& parameterName, int& value) const;

    /**
     * @brief Sets the value of an _Enum-type device parameter by inputting the integer value.
     * See @ref Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found or read only,
     *  please check the input parameter name.\n
     *  @ref ErrorStatus.MMIND_STATUS_OUT_OF_RANGE_ERROR Invalid enum input.\n
     */
    ErrorStatus setEnumValue(const std::string& parameterName, int value);

    /**
     * @brief Gets the current value of an _Enum-type device parameter in the form of a string. See
     * @ref Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getEnumValue(const std::string& parameterName, std::string& valueStr) const;

    /**
     * @brief Sets the value of an _Enum-type device parameter by inputting a string. See @ref
     * Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found or read only, please
     *  check the input parameter name.\n
     *  @ref ErrorStatus.MMIND_STATUS_OUT_OF_RANGE_ERROR Invalid enum input.\n
     */
    ErrorStatus setEnumValue(const std::string& parameterName, const std::string& value);

    /**
     * @brief Gets the current value of an _Roi-type device parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the input
     *  parameter name.\n
     */
    ErrorStatus getRoiValue(const std::string& parameterName, ROI& value) const;

    /**
     * @brief Sets the value of an _Roi-type device parameter. See @ref Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found or read only, please
     *  check the input parameter name.\n
     *  @ref ErrorStatus.MMIND_STATUS_OUT_OF_RANGE_ERROR Invalid parameter input.\n
     */
    ErrorStatus setRoiValue(const std::string& parameterName, const ROI& value);

    /**
     * @brief Gets the current value of an _ProfileRoi-type device parameter. See @ref Parameter
     * for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the input
     *  parameter name.\n
     */
    ErrorStatus getProfileRoiValue(const std::string& parameterName, ProfileROI& value) const;

    /**
     * @brief Sets the value of an _ProfileRoi-type device parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found or read only, please
     *  check the input parameter name.\n
     *  @ref ErrorStatus.MMIND_STATUS_OUT_OF_RANGE_ERROR Invalid parameter input.\n
     */
    ErrorStatus setProfileRoiValue(const std::string& parameterName, const ProfileROI& value);

    /**
     * @brief Gets the current value of a _RoiArray-type device parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Parse response error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR There are errors in reply.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getRoiArrayValue(const std::string& parameterName, std::vector<ROI>& value) const;

    /**
     * @brief Gets the current value of a _FloatArray-type device parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getFloatArrayValue(const std::string& parameterName,
                                   std::vector<double>& value) const;

    /**
     * @brief Sets the value of a _FloatArray-type device parameter. See @ref Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found or read only,
     *  please check the input parameter name.\n
     *  @ref ErrorStatus.MMIND_STATUS_OUT_OF_RANGE_ERROR Invalid parameter input.\n
     */
    ErrorStatus setFloatArrayValue(const std::string& parameterName,
                                   const std::vector<double>& value);

    /**
     * @brief Gets the current value of a _Range-type device parameter. See @ref Parameter for
     * details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found, please check the
     *  input parameter name.\n
     */
    ErrorStatus getRangeValue(const std::string& parameterName, Range<int>& value) const;

    /**
     * @brief Sets the value of a _Range-type device parameter. See @ref Parameter for details.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_PARAMETER_ERROR Parameter not found or read only, please
     *  check the input parameter name.\n
     *  @ref ErrorStatus.MMIND_STATUS_OUT_OF_RANGE_ERROR Invalid parameter input.\n
     */
    ErrorStatus setRangeValue(const std::string& parameterName, const Range<int>& value);

private:
    std::shared_ptr<class SettingImpl> _impl;
    explicit UserSet(const std::shared_ptr<ZmqClientImpl>& impl);
    friend class UserSetManagerImpl;
};

} // namespace eye
} // namespace mmind
