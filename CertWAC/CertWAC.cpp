#include <Windows.h>
#include <tchar.h>
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

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
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
