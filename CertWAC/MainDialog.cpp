#include <string>
#include "ComputerCertificate.h"
#include "InstallInfo.h"
#include "MainDialog.h"

void InitDialog() noexcept
{
	//auto[RegistryError, ModifyPath] = GetMSIModifyPath();
	//auto ErrorCode = RegistryError.GetErrorCode();
	//if (ErrorCode > 0)
	//{
	//	MessageBox(DialogHandle, FormatErrorForPopup(
	//		ErrorCode, RegistryError.GetErrorMessage(), RegistryError.GetActivity()).c_str(),
	//		L"Installation Detection Error", MB_OK);
	//	PostQuitMessage(ErrorCode);
	//}
	//else
	//{

	//}
}

static std::wstring FormatErrorForPopup(const DWORD ErrorCode, const std::wstring& ErrorMessage, const std::wstring& Activity) noexcept
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
	case WM_INITDIALOG:
		//InitDialog();
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


MainDialog::MainDialog(HINSTANCE Instance) : AppInstance(Instance)
{
	DialogHandle = CreateDialogParam(AppInstance, MAKEINTRESOURCE(IDD_MAIN), 0, DialogProc, 0);
}