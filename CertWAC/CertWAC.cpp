#include <Windows.h>
#include <tchar.h>
#include <string>
#include <utility>
#include "resource.h"
#include "ComputerCertificate.h"
#include "InstallInfo.h"
#include "ErrorRecord.h"

std::wstring FormatErrorForDialog(const DWORD ErrorCode, const std::wstring& ErrorMessage, const std::wstring& Activity)
{
	return std::wstring{ L"Activity: " + Activity + L"\r\nErrorCode: " + std::to_wstring(ErrorCode) + L": " + ErrorMessage };
}

INT_PTR CALLBACK DialogProc(HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetComputerCertificates();
			return TRUE;
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


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	auto[RegistryError, MSICmdline] = GetMSIModifyPath();
	auto r = RegistryError.GetErrorCode();
	//if (RegistryError.GetErrorCode() > 0)
	//{
	//	MessageBox(NULL, FormatErrorForDialog(
	//		RegistryError.GetErrorCode(), RegistryError.GetErrorMessage(), RegistryError.GetActivity()).c_str(),
	//		L"Installation Detection Error", MB_OK);
	//}

	auto[CertError, CertificateList] = GetComputerCertificates();
	if (CertificateList.size())
	{
		MessageBox(NULL, L"We got some", L"Certs", MB_OK);
	}
	else
	{
		MessageBox(NULL, L"We got none", L"Certs", MB_OK);
	}

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
