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

void ActionDialog::CenterInParent() noexcept
{ // https://docs.microsoft.com/en-us/windows/desktop/dlgbox/using-dialog-boxes
	RECT ParentRect, ChildRect, TargetRect;
	GetWindowRect(HandleDialogMain, &ParentRect);
	GetWindowRect(HandleDialogAction, &ChildRect);
	CopyRect(&TargetRect, &ParentRect);
	OffsetRect(&ChildRect, -ChildRect.left, -ChildRect.top);
	OffsetRect(&TargetRect, -TargetRect.left, -TargetRect.top);
	OffsetRect(&TargetRect, -ChildRect.right, -ChildRect.bottom);
	SetWindowPos(HandleDialogAction, HWND_TOP,
		(ParentRect.left + (TargetRect.right / 2)),
		(ParentRect.top + (TargetRect.bottom / 2)),
		0, 0, SWP_NOSIZE);
}

INT_PTR CALLBACK ActionDialog::ThisDialogProc(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		SetWindowLongPtr(HandleDialogAction, GWL_USERDATA, (LONG_PTR)this);
		CenterInParent();
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
	:AppInstance(Instance), HandleDialogMain(Parent), DialogActions(Actions)
{
	DialogBoxParam(Instance, MAKEINTRESOURCE(IDD_ACTIONDIALOG), HandleDialogMain, &SharedDialogProc, (LPARAM)this);
	PostQuitMessage(0);
}