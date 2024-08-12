#include <string>
#include "ActionDialog.h"
#include "ErrorDialog.h"
#include "InstallInfo.h"
#include "MainDialog.h"
#include "StringUtility.h"

const wchar_t* MSIMiddle{ L" /qb SSL_CERTIFICATE_OPTION=installed SME_THUMBPRINT=" };
const wchar_t* ActivityPortValidation{ L"Verifying that port is in range of 1 to 65535" };
const wchar_t* ActionDescriptionRunningInstaller{ L"Running installer" };
const wchar_t* ActionDescriptionStartingService{ L"Starting WAC service" };
const wchar_t* ActionServiceController{ L"sc.exe" };	// do not localize
const wchar_t* ParamsServiceController{ L"start ServerManagementGateway" }; // do not localize

static std::pair<std::wstring, std::wstring> BuildMsiCommandSet(std::wstring RegistryPath, std::wstring Thumbprint)
{
	std::vector<std::wstring> MsiSet{ SplitString(L" ", RegistryPath) };
	if (MsiSet.size() > 1)
	{
		return std::make_pair(MsiSet[0], (MsiSet[1] + MSIMiddle + Thumbprint));
	}
	else
	{	// these values should always be "msiexec.exe[SPACE]...", but this might work if something changed
		return std::make_pair(RegistryPath, MSIMiddle + Thumbprint);
	}
}

void MainDialog::StartActions()
{
	BOOL PortReadResult;
	unsigned int Port{ GetDlgItemInt(HandleDialogMain, IDC_PORT, &PortReadResult, FALSE) };
	if (PortReadResult != TRUE || Port < 1 || Port > 65535)	// max valid range for IP ports
	{
		ErrorDialog(AppInstance, HandleDialogMain, ErrorRecord(ERROR_INVALID_PARAMETER, ActivityPortValidation));
		SendMessage(GetDlgItem(HandleDialogMain, IDC_PORT), EM_SETSEL, 0, -1);
		SetFocus(GetDlgItem(HandleDialogMain, IDC_PORT));		
		return;
	}
	ErrorRecord SetPortError{ SetListeningPort(Port) };
	if (SetPortError.ErrorCode() != ERROR_SUCCESS)
	{
		ErrorDialog(AppInstance, HandleDialogMain, SetPortError);
	}
	std::vector<std::tuple<std::wstring, std::wstring, std::wstring>> Actions;
	auto [MsiCmd, MsiParams] {BuildMsiCommandSet(CmdlineModifyPath, Thumbprint)};
	Actions.emplace_back(std::tuple(ActionDescriptionRunningInstaller, MsiCmd, MsiParams));
	Actions.emplace_back(std::tuple(ActionDescriptionStartingService, ActionServiceController, ParamsServiceController));
	ActionDialog ActionWindow(AppInstance, HandleDialogMain, Actions);
}

void MainDialog::Refresh()
{
	// load and process WAC installation information
	auto[InfoLoadError, ModifyPath, Port] {GetWACInstallInfo()};
	CmdlineModifyPath = ModifyPath;
	SetDlgItemText(HandleDialogMain, IDC_PORT, std::to_wstring(Port).c_str());
	if (InfoLoadError.ErrorCode() != ERROR_SUCCESS)
		ErrorDialog(AppInstance, HandleDialogMain, InfoLoadError);

	// load and process installed computer certificates
	auto[CertLoadError, CertificateList] {GetComputerCertificates()};
	Certificates = CertificateList;
	DisplayCertificateList();
	if (CertLoadError.ErrorCode() != ERROR_SUCCESS)
		ErrorDialog(AppInstance, HandleDialogMain, CertLoadError);
}