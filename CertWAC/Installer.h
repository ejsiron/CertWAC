#pragma once
#include <string>
#include "ErrorRecord.h"

ErrorRecord StartInstaller(const std::wstring ModifyPath, const std::wstring Thumbprint, const int Port);