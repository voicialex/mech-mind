#pragma once
#include <string>
#include "api_global.h"
#include "ErrorStatus.h"

namespace mmind {

namespace eye {

/**
 * @brief Exports the API logs
 * @param [in] The folder path used to store the exported logs.
 * @param [in] Whether to overwrite an existing file with the same name.
 * @return
 *  @ref ErrorStatus.MMIND_STATUS_SUCCESS Log export successful.\n
 *  @ref ErrorStatus.MMIND_STATUS_FILE_IO_ERROR. Failed to save the logs.\n
 */
MMIND_API_EXPORT ErrorStatus exportLogs(const std::string& dstPath, bool coverIfExist);

} // namespace eye

} // namespace mmind
