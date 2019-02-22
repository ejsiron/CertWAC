#include <sstream>
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
	MainDialog* AppDialog{ nullptr };
	if (uMessage == WM_INITDIALOG)
	{
		AppDialog = (MainDialog*)lParam;
		AppDialog->DialogHandle = hDialog;
	}
	else
		AppDialog = (MainDialog*)GetWindowLongPtr(hDialog, GWL_USERDATA);

	if (AppDialog)
		return AppDialog->ThisDialogProc(uMessage, wParam, lParam);
	return FALSE;
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
		case IDC_CERTLIST:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				DisplayCertificate();
			break;
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
		DisplayCertificateList();
		//if (InitResult != ERROR_SUCCESS)
		//	PostQuitMessage(InitResult);
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

void MainDialog::DisplayCertificateList() noexcept
{
	EnableDialogItem(IDC_CERTLIST, false);
	size_t ItemCount{ Certificates.size() };
	HWND CertList{ GetDlgItem(DialogHandle, IDC_CERTLIST) };
	SendMessage(CertList, CB_RESETCONTENT, 0, 0);

	if (ItemCount)
	{
		for (auto const& Certificate : Certificates)
		{
			SendMessage(CertList, CB_ADDSTRING, 0, (LPARAM)Certificate.FQDNFromSimpleRDN(Certificate.SubjectName()).c_str());
		}
		RECT CertListComboSize{ 0 };
		GetWindowRect(CertList, &CertListComboSize);
		SetWindowPos(CertList, 0, 0, 0, CertListComboSize.right - CertListComboSize.left,
			(SendMessage(CertList, CB_GETITEMHEIGHT, 0, 0) * ItemCount * 2), SWP_NOMOVE | SWP_NOZORDER);
		EnableDialogItem(IDC_CERTLIST, true);
	}
	else
		SendMessage(CertList, CB_ADDSTRING, 0, (LPARAM)L"No computer certificates found");
	SendMessage(CertList, CB_SETCURSEL, 0, 0);
}

void MainDialog::DisplayCertificate()
{
	std::wstring CertificateText{};
	HWND CertList{ GetDlgItem(DialogHandle, IDC_CERTLIST) };
	if (IsWindowEnabled(CertList) && Certificates.size())
	{
		ComputerCertificate& Certificate{ Certificates.at(SendMessage(CertList, CB_GETCURSEL, 0, 0)) };
		std::wstringstream CertificateDisplay{};
		CertificateDisplay << L"Subject: " << Certificate.SubjectName() << L"\r\n";
		CertificateDisplay << L"Issuer: " << Certificate.Issuer() << L"\r\n";
		if (Certificate.SubjectAlternateNames().size())
		{
			CertificateDisplay << L"Subject Alternate Names:\r\n";
			for (auto const& AlternateName : Certificate.SubjectAlternateNames())
			{
				CertificateDisplay << L'\t' << AlternateName << "\r\n";
			}
		}
		CertificateDisplay << L"Valid from: " << Certificate.ValidFrom() << L"\r\n";
		CertificateDisplay << L"Valid to: " << Certificate.ValidTo() << L"\r\n";
		CertificateDisplay << L"Thumbprint: " << Certificate.Thumbprint() << L"\r\n";
		CertificateDisplay.flush();
		CertificateText = CertificateDisplay.str();
	}
	SetDlgItemText(DialogHandle, IDC_CERTDETAILS, CertificateText.c_str());
}

const DWORD MainDialog::InitDialog() noexcept
{
	auto[RegistryError, ModifyPath] = GetMSIModifyPath();
	auto ErrorCode = RegistryError.GetErrorCode();
	if (ErrorCode > 0)
	{
#ifndef _DEBUG
		MessageBox(NULL, FormatErrorForPopup(
			ErrorCode, RegistryError.GetErrorMessage(), RegistryError.GetActivity()).c_str(),
			L"Installation Detection Error", MB_ICONERROR | MB_TASKMODAL);
#endif // DEBUG
	}
	else
		CmdlineModifyPath = ModifyPath;
	return ErrorCode;
}

MainDialog::MainDialog(HINSTANCE Instance) : AppInstance(Instance)
{
	auto[CertLoadError, CertificateList] = GetComputerCertificates();
	Certificates = CertificateList;
	DialogHandle = CreateDialogParam(AppInstance, MAKEINTRESOURCE(IDD_MAIN), 0, &SharedDialogProc, (LPARAM)this);
}