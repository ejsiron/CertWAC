#include <string>
#include <thread>
#include "Action.h"

void RunAction(const HWND Owner, const std::wstring& Action, const std::wstring& Params)
{
	MessageBox(NULL, Params.c_str(), Action.c_str(), MB_OK);
	ShellExecute(Owner, L"open", Action.c_str(), Params.c_str(), NULL, SW_SHOW);
	PostMessage(Owner, WMU_SUBTHREADCOMPLETE, 0, 0);
}

void StartAction(const HWND Owner, const std::wstring& Action, const std::wstring& Params)
{
	std::thread ActionThread(RunAction, Owner, Action, Params);
	ActionThread.detach();
}
