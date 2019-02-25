#include <map>
#include <sstream>
#include <string>
#include "ActionDialog.h"
#include "ComputerCertificate.h"
#include "ErrorRecord.h"
#include "InstallInfo.h"
#include "MainDialog.h"
#include "StringUtility.h"

static std::wstring FormatErrorForPopup(const DWORD ErrorCode, const std::wstring& ErrorMessage, const std::wstring& Activity) noexcept
{
	return std::wstring{ L"Activity: " + Activity + L"\r\nErrorCode: " + std::to_wstring(ErrorCode) + L": " + ErrorMessage };
}

static std::wstring SplitForOutput(const std::vector<std::wstring>& SplitInput)
{
	std::wstring Output{};
	if (SplitInput.size() > 1)
	{
		for (auto const& InputLine : SplitInput)
		{
			Output.append(L"\r\n-  " + InputLine);
		}
	}
	else if (SplitInput.size() == 1)
		Output = L" " + SplitInput[0];
	Output.append(L"\r\n");
	return Output;
}

static std::wstring SplitForOutput(const std::wstring& GluePattern, const std::wstring& Composite)
{
	return SplitForOutput(SplitString(GluePattern, Composite));
}

INT_PTR CALLBACK MainDialog::SharedDialogProc(HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	MainDialog* AppDialog{ nullptr };
	if (uMessage == WM_INITDIALOG)
	{
		AppDialog = (MainDialog*)lParam;
		AppDialog->HandleDialogMain = hDialog;
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
			StartActions();
			return TRUE;
		case IDCANCEL:
			SendMessage(HandleDialogMain, WM_CLOSE, 0, 0);
			return TRUE;
		case IDC_CERTLIST:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				DisplayCertificate();
			break;
		case IDC_REFRESH:
			GetWACInstallationInfo();
			DisplayCertificateList();
			break;
		}
		break;
		break;
	case WM_INITDIALOG:
	{
		SetLastError(ERROR_SUCCESS);
		SetWindowLongPtr(HandleDialogMain, GWL_USERDATA, (LONG_PTR)this);
		if (auto LastError = GetLastError())
		{
			MessageBox(HandleDialogMain, FormatErrorForPopup(LastError, ErrorRecord::GetErrorMessage(LastError).c_str(), L"Setting application information").c_str(), L"Startup Error", MB_OK);
			PostQuitMessage(LastError);
		}
		else
		{
			auto InitResult = InitDialog();
			SetPictureBoxImage(IDC_ICONWACINSTALLED, InitResult == ERROR_SUCCESS);
			DisplayCertificateList();
		}
	}
	break;
	case WM_CLOSE:
		DestroyWindow(HandleDialogMain);
		DeleteObject(TinyGreenBox);
		DeleteObject(TinyRedBox);
		return TRUE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

const DWORD MainDialog::GetWACInstallationInfo()
{
	auto[RegistryError, ModifyPath, Port] = GetWACInstallInfo();
	auto ErrorCode = RegistryError.GetErrorCode();
	CmdlineModifyPath = ModifyPath;
	SetDlgItemText(HandleDialogMain, IDC_PORT, std::to_wstring(Port).c_str());
	return ErrorCode;
}

void MainDialog::DisplayCertificateList() noexcept
{
	EnableDialogItem(IDC_CERTLIST, false);
	size_t ItemCount{ Certificates.size() };
	HWND CertList{ GetDlgItem(HandleDialogMain, IDC_CERTLIST) };
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
	DisplayCertificate();
}

void MainDialog::DisplayCertificate()
{
	std::wstring CertificateText{};
	bool StatusGreenWACDetection{ CmdlineModifyPath.size() > 0 };
	bool StatusGreenCertificateValid{ false };
	bool StatusGreenServerAuth{ false };
	bool StatusGreenPrivateKey{ false };

	HWND CertList{ GetDlgItem(HandleDialogMain, IDC_CERTLIST) };
	if (IsWindowEnabled(CertList) && Certificates.size())
	{
		ComputerCertificate& Certificate{ Certificates.at(SendMessage(CertList, CB_GETCURSEL, 0, 0)) };
		Thumbprint = Certificate.Thumbprint();
		std::wstringstream CertificateDisplay{};
		CertificateDisplay << L"Subject:" << SplitForOutput(L", ?", Certificate.SubjectName());
		CertificateDisplay << L"Issuer: " << SplitForOutput(L", ?", Certificate.Issuer());
		CertificateDisplay << L"Subject Alternate Names:" << SplitForOutput(Certificate.SubjectAlternateNames());
		CertificateDisplay << L"Valid from: " << Certificate.ValidFrom() << L"\r\n";
		CertificateDisplay << L"Valid to: " << Certificate.ValidTo() << L"\r\n";
		CertificateDisplay << L"Thumbprint: " << Thumbprint;
		CertificateText = CertificateDisplay.str();
		SetPictureBoxImage(IDC_ICONCERTVALID, (StatusGreenCertificateValid = Certificate.IsWithinValidityPeriod()));
		SetPictureBoxImage(IDC_ICONSERVAUTHALLOWED, (StatusGreenServerAuth = Certificate.EKUServerAuthentication()));
		SetPictureBoxImage(IDC_ICONHASPRIVATEKEY, (StatusGreenPrivateKey = Certificate.HasPrivateKey()));
	}
	SetDlgItemText(HandleDialogMain, IDC_CERTDETAILS, CertificateText.c_str());
	EnableDialogItem(IDOK, StatusGreenWACDetection && StatusGreenCertificateValid &&
		StatusGreenServerAuth && StatusGreenPrivateKey);
}

void MainDialog::SetPictureBoxImage(const INT PictureBoxID, const bool Good)
{
	HBITMAP SelectedImage = Good ? TinyGreenBox : TinyRedBox;
	SendDlgItemMessage(HandleDialogMain, PictureBoxID, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)SelectedImage);
}

static std::pair<std::wstring, std::wstring> BuildMsiCommandSet(std::wstring RegistryPath, std::wstring Thumbprint)
{
	const std::wstring MSIMiddle{ L" /qb SSL_CERTIFICATE_OPTION=installed SME_THUMBPRINT=" };
	std::vector<std::wstring> MsiSet{ SplitString(L" ", RegistryPath) };
	return std::make_pair(MsiSet[0], (MsiSet[1] + MSIMiddle + Thumbprint));
}

void MainDialog::StartActions()
{
	BOOL PortReadResult;
	unsigned int Port{ GetDlgItemInt(HandleDialogMain, IDC_PORT, &PortReadResult, FALSE) };
	if (PortReadResult != TRUE || Port < 1 || Port > 65535)	// max valid range for IP ports
	{
		MessageBox(HandleDialogMain, L"Port must be greater than 0 and less than 65535", L"Port Error", MB_ICONERROR);
		SetFocus(GetDlgItem(HandleDialogMain, IDC_PORT));
		return;
	}
	ErrorRecord SetPortError{ SetListeningPort(Port) };
	if(SetPortError.GetErrorCode() != ERROR_SUCCESS)
		MessageBox(HandleDialogMain, FormatErrorForPopup(SetPortError.GetErrorCode(), SetPortError.GetErrorMessage(), SetPortError.GetActivity()).c_str(), L"Port Error", MB_ICONERROR);
	std::vector<std::tuple<std::wstring, std::wstring, std::wstring>> Actions;
	auto [MsiCmd, MsiParams] {BuildMsiCommandSet(CmdlineModifyPath, Thumbprint)};
	Actions.emplace_back(std::tuple(L"Running installer", MsiCmd, MsiParams));
	Actions.emplace_back(std::tuple(L"Starting service", L"sc", L"start ServerManagementGateway"));
	ActionDialog ActionWindow(AppInstance, HandleDialogMain, Actions);
}

const DWORD MainDialog::InitDialog() noexcept
{
	HINSTANCE Instance{ GetModuleHandle(NULL) };
	AppIcon = LoadIcon(Instance, MAKEINTRESOURCE(IDI_CERTWAC));
	TinyGreenBox = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_TINYGREENBOX));
	TinyRedBox = LoadBitmap(Instance, MAKEINTRESOURCE(IDB_TINYREDBOX));
	SendMessage(HandleDialogMain, WM_SETICON, ICON_SMALL, (LPARAM)AppIcon);
	SetPictureBoxImage(IDC_ICONWACINSTALLED, false);
	SetPictureBoxImage(IDC_ICONCERTVALID, false);
	SetPictureBoxImage(IDC_ICONSERVAUTHALLOWED, false);
	SetPictureBoxImage(IDC_ICONHASPRIVATEKEY, false);
	auto[CertLoadError, CertificateList] = GetComputerCertificates();
	Certificates = CertificateList;
	auto ErrorCode = GetWACInstallationInfo();
	return ErrorCode;
}

MainDialog::MainDialog(HINSTANCE Instance) : AppInstance(Instance)
{
	HandleDialogMain = CreateDialogParam(AppInstance, MAKEINTRESOURCE(IDD_MAIN), 0, &SharedDialogProc, (LPARAM)this);
}