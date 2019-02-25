#pragma once
#include <Windows.h>
#include <string>
#include <vector>

#define WMU_SUBTHREADCOMPLETE (WM_APP + 1)

// Spawns a separate thread to execute Action with Params.
// Sends WMU_SUBTHREADCOMPLETE message to Owner when Action completes.
// - will NOT work with COM objects
void StartAction(const HWND Owner, const std::wstring& Action, const std::wstring& Params);