#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "ErrorRecord.h"

#define WMU_SUBTHREADCOMPLETE (WM_APP + 1)

void StartAction(const HWND Owner, const std::wstring& Action, const std::wstring& Params);