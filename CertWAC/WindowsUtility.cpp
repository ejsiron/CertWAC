#include "WindowsUtility.h"

void CenterInParent(HWND Parent, HWND Child) noexcept
{ // https://docs.microsoft.com/en-us/windows/desktop/dlgbox/using-dialog-boxes
	RECT ParentRect, ChildRect, TargetRect;
	GetWindowRect(Parent, &ParentRect);
	GetWindowRect(Child, &ChildRect);
	CopyRect(&TargetRect, &ParentRect);
	OffsetRect(&ChildRect, -ChildRect.left, -ChildRect.top);
	OffsetRect(&TargetRect, -TargetRect.left, -TargetRect.top);
	OffsetRect(&TargetRect, -ChildRect.right, -ChildRect.bottom);
	SetWindowPos(Child, HWND_TOP,
		(ParentRect.left + (TargetRect.right / 2)),
		(ParentRect.top + (TargetRect.bottom / 2)),
		0, 0, SWP_NOSIZE);
}