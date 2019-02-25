#pragma once
#include <string>
#include <vector>
#include "WindowsTarget.h"

#define WMU_SUBTHREADCOMPLETE (WM_APP + 1)

// Spawns a separate thread to execute Action with Params.
// Sends WMU_SUBTHREADCOMPLETE message to Owner when Action completes.
void StartAction(const HWND Owner, const std::wstring& Action, const std::wstring& Params);