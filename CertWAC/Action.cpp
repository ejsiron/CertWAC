#include <thread>
#include <Windows.h>
#include "Action.h"

void RunAction(const HWND Owner, const std::wstring& Action, const std::wstring& Params)
{
	SHELLEXECUTEINFO ActionInfo;
	ActionInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ActionInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ActionInfo.hwnd = Owner;
	ActionInfo.lpVerb = L"open";
	ActionInfo.lpFile = Action.c_str();
	ActionInfo.lpParameters = Params.c_str();
	ActionInfo.lpDirectory = NULL;
	ActionInfo.nShow = SW_HIDE;
	ActionInfo.hInstApp = NULL;
	ShellExecuteEx(&ActionInfo);
	if ((int)ActionInfo.hInstApp >= 32) // https://docs.microsoft.com/windows/desktop/api/shellapi/ns-shellapi-_shellexecuteinfoa
	{
		WaitForSingleObject(ActionInfo.hProcess, INFINITE);
	}
	PostMessage(Owner, WMU_SUBTHREADCOMPLETE, 0, 0);
}

void StartAction(const HWND Owner, const std::wstring& Action, const std::wstring& Params)
{
	std::thread ActionThread(RunAction, Owner, Action, Params);
	ActionThread.detach();
}
