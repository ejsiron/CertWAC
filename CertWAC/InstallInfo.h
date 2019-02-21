#pragma once

#include <string>
#include "ErrorRecord.h"

std::pair<ErrorRecord, std::wstring> GetMSIModifyPath() noexcept;