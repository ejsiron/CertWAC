#include <string>
#include "ActionDialog.h"
#include "ErrorDialog.h"
#include "InstallInfo.h"
#include "MainDialog.h"
#include "StringUtility.h"

const wchar_t* MSIMiddle{ L" /qb SSL_CERTIFICATE_OPTION=installed SME_THUMBPRINT=" };

const DWORD MainDialog::GetWACInstallationInfo()
{
	auto[RegistryError, ModifyPath, Port] = GetWACInstallInfo();
	auto ErrorCode = RegistryError.ErrorCode();
	CmdlineModifyPath = ModifyPath;
	SetDlgItemText(HandleDialogMain, IDC_PORT, std::to_wstring(Port).c_str());
	return ErrorCode;
}

static std::pair<std::wstring, std::wstring> BuildMsiCommandSet(std::wstring RegistryPath, std::wstring Thumbprint)
{
	std::vector<std::wstring> MsiSet{ SplitString(L" ", RegistryPath) };
	if (MsiSet.size() > 1)
	{
		return std::make_pair(MsiSet[0], (MsiSet[1] + MSIMiddle + Thumbprint));
	}
	else
	{	// these values should always be "msiexec.exe ...", but this might work if something changed
		return std::make_pair(RegistryPath, MSIMiddle + Thumbprint);
	}
}

void MainDialog::StartActions()
{
	BOOL PortReadResult;
	unsigned int Port{ GetDlgItemInt(HandleDialogMain, IDC_PORT, &PortReadResult, FALSE) };
	if (PortReadResult != TRUE || Port < 1 || Port > 65535)	// max valid range for IP ports
	{
		ErrorDialog(AppInstance, HandleDialogMain, ErrorRecord(ERROR_INVALID_PARAMETER, L"Verifying that port is in range of 1 to 65535"));
		SetFocus(GetDlgItem(HandleDialogMain, IDC_PORT));
		return;
	}
	ErrorRecord SetPortError{ SetListeningPort(Port) };
	if (SetPortError.ErrorCode() != ERROR_SUCCESS)
		ErrorDialog(AppInstance, HandleDialogMain, SetPortError);
	std::vector<std::tuple<std::wstring, std::wstring, std::wstring>> Actions;
	auto [MsiCmd, MsiParams] {BuildMsiCommandSet(CmdlineModifyPath, Thumbprint)};
	Actions.emplace_back(std::tuple(L"Running installer", MsiCmd, MsiParams));
	Actions.emplace_back(std::tuple(L"Starting service", L"sc", L"start ServerManagementGateway"));
	ActionDialog ActionWindow(AppInstance, HandleDialogMain, Actions);
}