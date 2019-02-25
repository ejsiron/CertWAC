#include <Windows.h>
#include <string>
#include "ErrorDialog.h"
#include "ErrorRecord.h"

INT_PTR CALLBACK ErrorDialog::SharedDialogProc(HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	ErrorDialog* ThisErrorDialog{ nullptr };
	if (uMessage == WM_INITDIALOG)
	{
		ThisErrorDialog = (ErrorDialog*)lParam;
		ThisErrorDialog->HandleDialogError = hDialog;
	}
	else
		ThisErrorDialog = (ErrorDialog*)GetWindowLongPtr(hDialog, GWL_USERDATA);

	if (ThisErrorDialog)
		return ThisErrorDialog->ThisDialogProc(uMessage, wParam, lParam);
	return FALSE;
}

INT_PTR CALLBACK ErrorDialog::ThisDialogProc(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(HandleDialogError, GWL_USERDATA, (LONG_PTR)this);
		SetDlgItemText(HandleDialogError, IDC_ERRORCODE, std::to_wstring(Error.ErrorCode()).c_str());
		SetDlgItemText(HandleDialogError, IDC_ERRORMESSAGE, Error.ErrorMessage().c_str());
		SetDlgItemText(HandleDialogError, IDC_ACTIVITYMESSAGE, Error.Activity().c_str());
		SetFocus(GetDlgItem(HandleDialogError, IDOK));
		break;
	case WM_CLOSE:
		DestroyWindow(HandleDialogError);
		return TRUE;
	}
	return FALSE;
}

static std::wstring FormatErrorForPopup(const DWORD ErrorCode, const std::wstring& ErrorMessage, const std::wstring& Activity) noexcept
{
	return std::wstring{ L"Activity: " + Activity + L"\r\nErrorCode: " + std::to_wstring(ErrorCode) + L": " + ErrorMessage };
}

ErrorDialog::ErrorDialog(const HINSTANCE Instance, const HWND Parent, const ErrorRecord& ErrorDetails)
	:Error(ErrorDetails)
{
	DialogBoxParam(Instance, MAKEINTRESOURCE(IDD_ERRORDIALOG), Parent, &SharedDialogProc, (LPARAM)this);
}