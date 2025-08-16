#pragma once
#include <memory>
#include "api_global.h"
#include "UserSet.h"

namespace mmind {
class ZmqClientImpl;

namespace eye {

/**
 * @brief Manages device user sets.
 */
class MMIND_API_EXPORT UserSetManager
{
public:
    /**
     * @brief Destructor.
     */
    virtual ~UserSetManager();

    /**
     * @brief Returns a reference to the UserSet object.
     * @return See @ref UserSet for details.
     */
    UserSet& currentUserSet() const;

    /**
     * @brief Gets the names of all available device user sets.
     * @param [out] userSets The array of all user set names.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     */
    ErrorStatus getAllUserSetNames(std::vector<std::string>& userSets) const;

    /**
     * @brief Selects a device user set to be associated with unsaved parameter adjustments.
     * @param [in] userSetName The name of the user set to be selected.
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
    ErrorStatus selectUserSet(const std::string& userSetName) const;

    /**
     * @brief Adds a user set to the device.
     * @param [in] userSetName The name of the user set to be added.
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
    ErrorStatus addUserSet(const std::string& userSetName) const;

    /**
     * @brief Deletes the specified device user set.
     * @param [in] userSetName The name of the user set to be deleted.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_INPUT_ERROR The user set name entered does not
     * exist.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     */
    ErrorStatus deleteUserSet(const std::string& userSetName) const;

    /**
     * @brief Exports all device user sets to a JSON file.
     * @param [in] fileName The name of the JSON file.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Failed to save the JSON file.\n
     */
    ErrorStatus saveToFile(const std::string& fileName) const;

    /**
     * @brief Imports device user sets from a JSON file and overwrites existing
     * user sets on the device.
     * @param [in] fileName The name of the JSON file.
     * @return
     *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Success.\n
     *  @ref ErrorStatus.MMIND_STATUS_INVALID_DEVICE Invalid device handle.\n
     *  @ref ErrorStatus.MMIND_STATUS_DEVICE_OFFLINE Device disconnected.\n
     *  @ref ErrorStatus.MMIND_STATUS_TIMEOUT_ERROR Timeout error.\n
     *  @ref ErrorStatus.MMIND_STATUS_RESPONSE_PARSE_ERROR Response parsing error.\n
     *  @ref ErrorStatus.MMIND_STATUS_REPLY_WITH_ERROR The reply from the device contains errors.\n
     *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR Wrong file name passed, failed to save or open
     * the JSON file, or incorrect file format.\n
     */
    ErrorStatus loadFromFile(const std::string& fileName);

private:
    std::shared_ptr<class UserSetManagerImpl> _impl;
    explicit UserSetManager(const std::shared_ptr<ZmqClientImpl>& client);
    friend class CameraImpl;
    friend class ProfilerImpl;
};

} // namespace eye
} // namespace mmind
