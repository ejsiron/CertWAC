#include <Windows.h>
#include <tchar.h>
#include <utility>
#include "resource.h"

INT_PTR CALLBACK DialogProc(HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			SendMessage(hDialog, WM_CLOSE, 0, 0);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hDialog);
		return TRUE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

std::pair<bool, DWORD> IsAdmin()
{
	try
	{
		HANDLE hCurrentProcess{ GetCurrentProcess() };
		HANDLE hProcessToken{ 0 };
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcessToken))
			throw;

		TOKEN_ELEVATION_TYPE ElevationType;
		DWORD TokenSize;
		if (!GetTokenInformation(hProcessToken, TokenElevationType, &ElevationType, sizeof(ElevationType), &TokenSize))
			throw;

		bool IsAdmin{ ElevationType == TOKEN_ELEVATION_TYPE::TokenElevationTypeFull };
		return std::make_pair(IsAdmin, ERROR_SUCCESS);

		//TOKEN_ELEVATION Elevation;
		//if (!GetTokenInformation(hProcessToken, TokenElevation, &Elevation, sizeof(Elevation), &TokenSize))
		//	throw;
	}
	catch (...)
	{
		return std::make_pair(false, GetLastError());
	}
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	const auto[bIsAdmin, dwError] { IsAdmin()};
	if (dwError != ERROR_SUCCESS)
		OutputDebugStringW(L"Error!");
	if (bIsAdmin)
		OutputDebugStringW(L"Is admin");
	else
		OutputDebugStringW(L"Not admin");
	HWND hMainDialog{ CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, DialogProc, 0) };
	ShowWindow(hMainDialog, nCmdShow);
	MSG msg;
	BOOL resGetMessage;
	while (resGetMessage = GetMessage(&msg, 0, 0, 0) > 0)
	{
		if (!IsDialogMessage(hMainDialog, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return resGetMessage;
}
