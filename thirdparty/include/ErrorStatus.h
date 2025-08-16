#pragma once
#include <string>
#include <iostream>

namespace mmind {

namespace eye {
/**
 * @brief Describes the types of errors.
 */
struct ErrorStatus
{
    /**
     * @brief Describes the error codes.
     */
    enum ErrorCode {
        MMIND_STATUS_SUCCESS = 0, ///< Success.
        MMIND_STATUS_INVALID_DEVICE =
            -1, ///< A Camera or Profiler object not connected to an actual device is used, or an IP
                ///< address of a non Mech-Eye device is input to the connect() method.
        MMIND_STATUS_DEVICE_OFFLINE = -2, ///< Device is offline. Network issue may be present.
        MMIND_STATUS_NO_SUPPORT_ERROR =
            -3, ///< The operation is not supported. Possible reasons include: 1. The firmware
                ///< version is inconsistent with the version of Mech-Eye API; 2. The device model
                ///< does not provide the corresponding function.
        MMIND_STATUS_OUT_OF_RANGE_ERROR = -4, ///< The input device parameter value exceeds the
                                              ///< settable value range.
        MMIND_STATUS_PARAMETER_ERROR =
            -5, ///< The input device parameter name does not exist, or the data type of the device
                ///< parameter does not match the method.
        MMIND_STATUS_NO_DATA_ERROR =
            -6, ///< The image data is empty. Some error may have occurred on the device.
        MMIND_STATUS_INVALID_INPUT_ERROR =
            -7, ///< An invalid input not covered by other error types is provided, such as a
                ///< nonexisting name of device user set.
        MMIND_STATUS_FILE_IO_ERROR =
            -8, ///< An error occurred when an operation related to reading or writing a file was
                ///< executed, such as invalid filename and incorrect file type.
        MMIND_STATUS_TIMEOUT_ERROR =
            -9, ///< The set timeout period exceeded when an operation such as connecting
                ///< to the device and retrieving data was performed.
        MMIND_HANDEYE_CALIBRATION_EXECUTION_ERROR =
            -10, ///< An error occurred while executing the hand-eye calibration.
        MMIND_STATUS_REPLY_WITH_ERROR = -11, ///< The reply from the device contains errors.

        MMIND_STATUS_ACQUISITION_TRIGGER_WAIT = -12, ///< Data acquisition has not been started.

        MMIND_STATUS_DEVICE_BUSY = -13,          ///< The callback function was registered again
                                                 ///< before the ongoing data acquisition stopped.
        MMIND_STATUS_INVALID_CALLBACKFUNC = -14, ///< The registered callback function is invalid.

        MMIND_STATUS_RESPONSE_PARSE_ERROR = -15, ///< It is error to parse the response from device.

        MMIND_STATUS_PROFILE_POST_PROCESS_ERROR =
            -16, ///< An error occurred while profile post process.

        MMIND_STATUS_MESSAGE_CHANNEL_ERROR = -17, ///< An error occurred when the device was trying
                                                  ///< to establish/close the message channel.

        MMIND_STATUS_DUPLICATE_REGISTRATION =
            -18, ///< A callback function for the specific event has already been registered. The
                 ///< same event cannot have more than one callback function registered
                 ///< simultaneously.
        MMIND_STATUS_MESSAGE_CHANNEL_OCCUPPIED =
            -19, ///< Another client program has already established a message channel with the
                 ///< device.
        MMIND_STATUS_BUFFER_FULL = -20, ///< The image buffer is full.
    };
    /**
     * @brief Default constructor.
     */
    ErrorStatus() = default;

    /**
     * @brief Constructor.
     */
    ErrorStatus(ErrorCode code, const std::string& message)
        : errorCode(code), errorDescription(message)
    {
    }

    /**
     * @brief Returns true if the operation succeeded.
     */
    bool isOK() const { return errorCode == MMIND_STATUS_SUCCESS; }

    /**
     * Error code.
     */
    ErrorCode errorCode{MMIND_STATUS_SUCCESS};

    /**
     * Detailed error message.
     */
    std::string errorDescription;
};

/**
 * @brief Prints the error code and its description.
 */
inline void showError(const mmind::eye::ErrorStatus& status, const std::string& successMessage = {})
{
    if (status.isOK()) {
        if (!successMessage.empty())
            std::cout << successMessage << std::endl;
        return;
    }
    std::cout << "Error Code : " << status.errorCode
              << ", Error Description: " << status.errorDescription << std::endl;
}

} // namespace eye

} // namespace mmind
