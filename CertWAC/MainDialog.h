#pragma once
#include <Windows.h>
#include "ComputerCertificate.h"
#include "resource.h"

class MainDialog
{
private:
	/* Windows components */
	HINSTANCE AppInstance{ 0 };
	HWND DialogHandle{ 0 };
	HICON AppIcon{ 0 };
	static INT_PTR CALLBACK SharedDialogProc(HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam);
	INT_PTR CALLBACK ThisDialogProc(UINT uMessage, WPARAM wParam, LPARAM lParam);
	HBITMAP TinyGreenBox{ 0 };
	HBITMAP TinyRedBox{ 0 };

	/* Controller components */
	std::vector<ComputerCertificate> Certificates{};
	int ListeningPort;
	std::wstring CmdlineModifyPath{};
	const DWORD GetWACInstallationInfo();
	const DWORD InitDialog() noexcept;
	void EnableDialogItem(const int DialogItem, const bool Enable = true) const noexcept { EnableWindow(GetDlgItem(DialogHandle, DialogItem), Enable); };
	void DisplayCertificateList() noexcept;
	void DisplayCertificate();
	void SetPictureBoxImage(const INT PictureBoxID, const bool Good = true);

public:
	MainDialog(HINSTANCE Instance);
	MainDialog(const MainDialog&) = delete;
	MainDialog& operator=(const MainDialog&) = delete;
	MainDialog(MainDialog&&) = default;
	MainDialog& operator=(MainDialog&&) = default;
	void Show(int ShowCommand) const noexcept { ShowWindow(DialogHandle, ShowCommand); }
	const HWND GetWindowHandle() const noexcept { return DialogHandle; }
	void AssignModifyPath(const std::wstring& NewPath) { CmdlineModifyPath = NewPath; }
};