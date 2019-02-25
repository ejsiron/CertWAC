#pragma once

#include <string>
#include "ErrorRecord.h"

// Retrieves installation information for Windows Admin Center
// - ErrorRecord: the last error message the function encountered
// - wstring: WAC's MSI ModifyPath setting
// - int: current WAC port number
std::tuple<ErrorRecord, std::wstring, int> GetWACInstallInfo() noexcept;

ErrorRecord SetListeningPort(const unsigned int Port) noexcept;