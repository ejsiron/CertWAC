#include "ActionDialog.h"
#include "Action.h"

INT_PTR CALLBACK ActionDialog::SharedDialogProc(HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	ActionDialog* ThisActionDialog{ nullptr };
	if (uMessage == WM_INITDIALOG)
	{
		ThisActionDialog = (ActionDialog*)lParam;
		ThisActionDialog->HandleDialogAction = hDialog;
	}
	else
		ThisActionDialog = (ActionDialog*)GetWindowLongPtr(hDialog, GWL_USERDATA);

	if (ThisActionDialog)
		return ThisActionDialog->ThisDialogProc(uMessage, wParam, lParam);
	return FALSE;
}

void ActionDialog::UpdateStatus(const std::wstring& Message)
{
	SetDlgItemText(HandleDialogAction, IDC_STATUSDISPLAY, Message.c_str());
}

void ActionDialog::InitiateAction()
{
	if (CompletedActions < DialogActions.size())
	{
		auto [Description, Command, Params] {DialogActions[CompletedActions]};
		UpdateStatus(Description);
		StartAction(HandleDialogAction, Command, Params);
	}
	else
	{
		EndDialog(HandleDialogAction, 0);
	}
}

void ActionDialog::CenterWindow()
{
	HWND Parent{ GetParent(HandleDialogAction) };
	RECT ParentRect;
	GetWindowRect(Parent, &ParentRect);
	RECT ThisRect;
	GetWindowRect(HandleDialogAction, &ThisRect);
	SetWindowPos(HandleDialogAction, NULL,
		((ParentRect.right - ParentRect.left - ThisRect.right - ThisRect.left) / 2),
		((ParentRect.bottom - ParentRect.top - ThisRect.bottom - ThisRect.top) / 2),
		0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE);
}

INT_PTR CALLBACK ActionDialog::ThisDialogProc(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(HandleDialogAction, GWL_USERDATA, (LONG_PTR)this);
		CenterWindow();
		InitiateAction();
		break;
	case WM_CLOSE:
		DestroyWindow(HandleDialogAction);
		return TRUE;
	case WMU_SUBTHREADCOMPLETE:
		++CompletedActions;
		InitiateAction();
		return TRUE;
	}
	return FALSE;
}

ActionDialog::ActionDialog(const HINSTANCE Instance, const HWND Parent, const std::vector<std::tuple<std::wstring, std::wstring, std::wstring>>& Actions)
	:AppInstance(Instance), DialogActions(Actions)
{
	DialogBoxParam(Instance, MAKEINTRESOURCE(IDD_ACTIONDIALOG), Parent, &SharedDialogProc, (LPARAM)this);
}