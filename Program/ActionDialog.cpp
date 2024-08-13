#include "ActionDialog.h"
#include "Action.h"
#include "WindowsUtility.h"

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

INT_PTR CALLBACK ActionDialog::ThisDialogProc(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(HandleDialogAction, GWL_USERDATA, (LONG_PTR)this);
		CenterInParent(HandleOwner, HandleDialogAction);
		InitiateAction();
		break;
	case WM_CLOSE:
		EndDialog(HandleDialogAction, 0);
		return TRUE;
	case WMU_SUBTHREADCOMPLETE:
		++CompletedActions;
		InitiateAction();
		return TRUE;
	}
	return FALSE;
}

ActionDialog::ActionDialog(const HINSTANCE Instance, const HWND Parent, const std::vector<std::tuple<std::wstring, std::wstring, std::wstring>>& Actions)
	:AppInstance(Instance), HandleOwner(Parent), DialogActions(Actions)
{
	DialogBoxParam(Instance, MAKEINTRESOURCE(IDD_ACTIONDIALOG), HandleOwner, &SharedDialogProc, (LPARAM)this);
	PostQuitMessage(0);
}