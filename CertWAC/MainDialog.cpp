#include <string>
#include "ComputerCertificate.h"
#include "ErrorRecord.h"
#include "InstallInfo.h"
#include "MainDialog.h"

static std::wstring FormatErrorForPopup(const DWORD ErrorCode, const std::wstring& ErrorMessage, const std::wstring& Activity) noexcept
{
	return std::wstring{ L"Activity: " + Activity + L"\r\nErrorCode: " + std::to_wstring(ErrorCode) + L": " + ErrorMessage };
}

INT_PTR CALLBACK MainDialog::SharedDialogProc(HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	SetLastError(ERROR_SUCCESS);
	MainDialog* AppDialog{ uMessage == WM_INITDIALOG ?
		(MainDialog*)lParam : (MainDialog*)GetWindowLongPtr(hDialog, GWL_USERDATA) };
	if (auto LastError = GetLastError() || !lParam)
	{
		LastError = LastError ? LastError : ERROR_INVALID_DATA;
		MessageBox(hDialog,
			FormatErrorForPopup(LastError, ErrorRecord::GetErrorMessage(LastError), L"Loading application data").c_str(),
			L"Processing Error", MB_ICONERROR);
		PostQuitMessage(ERROR_INVALID_DATA);
		return FALSE;
	}
	return AppDialog->ThisDialogProc(uMessage, wParam, lParam);
}

INT_PTR CALLBACK MainDialog::ThisDialogProc(UINT uMessage, WPARAM wParam, LPARAM lParam)
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
			SendMessage(DialogHandle, WM_CLOSE, 0, 0);
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
	{
		SetLastError(ERROR_SUCCESS);
		SetWindowLongPtr(DialogHandle, GWL_USERDATA, (LONG_PTR)this);
		if (auto LastError = GetLastError())
		{
			MessageBox(DialogHandle, FormatErrorForPopup(LastError, ErrorRecord::GetErrorMessage(LastError).c_str(), L"Setting application information").c_str(), L"Startup Error", MB_OK);
		}
		auto InitResult = InitDialog();
		if (InitResult != ERROR_SUCCESS)
			PostQuitMessage(InitResult);
	}
	break;
	case WM_CLOSE:
		DestroyWindow(DialogHandle);
		return TRUE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

const DWORD MainDialog::InitDialog()
{
	auto[RegistryError, ModifyPath] = GetMSIModifyPath();
	auto ErrorCode = RegistryError.GetErrorCode();
	if (ErrorCode > 0)
	{
		MessageBox(NULL, FormatErrorForPopup(
			ErrorCode, RegistryError.GetErrorMessage(), RegistryError.GetActivity()).c_str(),
			L"Installation Detection Error", MB_ICONERROR | MB_TASKMODAL);
	}
	else
		CmdlineModifyPath = ModifyPath;
	return ErrorCode;
}

MainDialog::MainDialog(HINSTANCE Instance) : AppInstance(Instance)
{
	DialogHandle = CreateDialogParam(AppInstance, MAKEINTRESOURCE(IDD_MAIN), 0, &SharedDialogProc, (LPARAM)this);
}