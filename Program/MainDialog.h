#pragma once
#include <Windows.h>
#include "ComputerCertificate.h"
#include "resource.h"

class MainDialog
{
private:
	/* Windows components */
	HINSTANCE AppInstance{ 0 };
	HWND HandleDialogMain{ 0 };
	HICON AppIcon{ 0 };
	static INT_PTR CALLBACK SharedDialogProc(HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam);
	INT_PTR CALLBACK ThisDialogProc(UINT uMessage, WPARAM wParam, LPARAM lParam);

	/* Controller components */
	std::vector<ComputerCertificate> Certificates{};
	std::wstring CmdlineModifyPath{};
	std::wstring Thumbprint{};
	const void InitDialog() noexcept;
	void Refresh();
	void EnableDialogItem(const int DialogItem, const bool Enable = true) const noexcept { EnableWindow(GetDlgItem(HandleDialogMain, DialogItem), Enable); };
	void DisplayCertificateList() noexcept;
	void DisplayCertificate();
	void StartActions();

public:
	MainDialog(HINSTANCE Instance);
	MainDialog(const MainDialog&) = delete;
	MainDialog& operator=(const MainDialog&) = delete;
	MainDialog(MainDialog&&) = default;
	MainDialog& operator=(MainDialog&&) = default;
	~MainDialog() = default;

	HWND Show(int ShowCommand) const noexcept;
	void AssignModifyPath(const std::wstring& NewPath) { CmdlineModifyPath = NewPath; }
};