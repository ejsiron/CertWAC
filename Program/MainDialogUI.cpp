#include <sstream>
#include <string>
#include "ComputerCertificate.h"
#include "ErrorDialog.h"
#include "ErrorRecord.h"
#include "InstallInfo.h"
#include "MainDialog.h"
#include "StringUtility.h"

const wchar_t* NewLine{ L"\r\n" };
const wchar_t* ActivitySetupEnvironment{ L"Setting up application environment" };
const wchar_t* NoCertsFound{ L"No computer certificates found" };
const wchar_t* RegexPatternCommaWithOptionalSpace{ L", ?" };
const wchar_t* FieldSubject{ L"Subject:" }; // program will determine whether or not to prepend a L" "
const wchar_t* FieldIssuer{ L"Issuer: " };
const wchar_t* FieldSAN{ L"Subject Alternate Names:" }; // program will determine whether or not to prepend a L" "
const wchar_t* FieldValidFrom{ L"Valid from: " };
const wchar_t* FieldValidTo{ L"Valid to: " };
const wchar_t* FieldThumbprint{ L"Thumbprint: " };

static std::wstring ConvertSplitForDisplay(const std::vector<std::wstring>& SplitInput)
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
	Output.append(NewLine);
	return Output;
}

static std::wstring ConvertSplitForDisplay(const std::wstring& GluePattern, const std::wstring& Composite)
{
	return ConvertSplitForDisplay(SplitString(GluePattern, Composite));
}

HWND MainDialog::Show(int ShowCommand) const noexcept
{
	ShowWindow(HandleDialogMain, ShowCommand);
	return HandleDialogMain;
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
			return TRUE;
		case IDC_REFRESH:
			Refresh();
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
	{
		SetLastError(ERROR_SUCCESS);
		SetWindowLongPtr(HandleDialogMain, GWL_USERDATA, (LONG_PTR)this);
		if (auto LastError = GetLastError())
		{
			ErrorDialog(AppInstance, HandleDialogMain, ErrorRecord(LastError, ActivitySetupEnvironment));
			PostQuitMessage(LastError);
		}
		else
		{
			InitDialog();
		}
	}
	break;
	case WM_CLOSE:
		DestroyWindow(HandleDialogMain);
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
		SendMessage(CertList, CB_ADDSTRING, 0, (LPARAM)NoCertsFound);
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
		CertificateDisplay << FieldSubject << ConvertSplitForDisplay(RegexPatternCommaWithOptionalSpace, Certificate.SubjectName());
		CertificateDisplay << FieldIssuer << ConvertSplitForDisplay(RegexPatternCommaWithOptionalSpace, Certificate.Issuer());
		CertificateDisplay << FieldSAN << ConvertSplitForDisplay(RegexPatternCommaWithOptionalSpace, Certificate.SubjectAlternateNames());
		CertificateDisplay << FieldValidFrom << Certificate.ValidFrom() << NewLine;
		CertificateDisplay << FieldValidTo << Certificate.ValidTo() << NewLine;
		CertificateDisplay << FieldThumbprint << Thumbprint;
		CertificateText = CertificateDisplay.str();
		StatusGreenCertificateValid = Certificate.IsWithinValidityPeriod();
		StatusGreenServerAuth = Certificate.HasServerAuthentication();
		StatusGreenPrivateKey = Certificate.HasPrivateKey();
	}
	SetDlgItemText(HandleDialogMain, IDC_CERTDETAILS, CertificateText.c_str());
	EnableDialogItem(IDOK, StatusGreenWACDetection && StatusGreenCertificateValid &&
		StatusGreenServerAuth && StatusGreenPrivateKey);
}

const void MainDialog::InitDialog() noexcept
{
	Refresh();
}

MainDialog::MainDialog(HINSTANCE Instance) : AppInstance(Instance)
{
	HandleDialogMain = CreateDialogParam(AppInstance, MAKEINTRESOURCE(IDD_MAIN), 0, &SharedDialogProc, (LPARAM)this);
}