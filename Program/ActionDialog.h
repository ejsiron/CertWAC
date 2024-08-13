#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include "resource.h"

// Small non-interactive dialog that places itself directly above and in the middle of its owner.
// Displays current task in a static control.
class ActionDialog
{
private:
	/* Windows components */
	const HINSTANCE AppInstance{ 0 };
	const HWND HandleOwner{ 0 };
	HWND HandleDialogAction{ 0 };
	static INT_PTR CALLBACK SharedDialogProc(HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam);
	INT_PTR CALLBACK ThisDialogProc(UINT uMessage, WPARAM wParam, LPARAM lParam);

	/* Controller */
	const std::vector<std::tuple<std::wstring, std::wstring, std::wstring>>& DialogActions;
	void UpdateStatus(const std::wstring& Message);
	size_t CompletedActions{ 0 };
	void InitiateAction();
public:
	ActionDialog(const HINSTANCE Instance, const HWND Parent, const std::vector<std::tuple<std::wstring, std::wstring, std::wstring>>& Actions);
	ActionDialog(const ActionDialog&) = delete;
	ActionDialog& operator=(const ActionDialog&) = delete;
	ActionDialog(ActionDialog&&) = delete;
	ActionDialog& operator=(ActionDialog&&) = delete;
};