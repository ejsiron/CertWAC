#pragma once
#include <Windows.h>
#include "ComputerCertificate.h"
#include "resource.h"

class MainDialog
{
private:
	HINSTANCE AppInstance;
	HWND DialogHandle;
	std::vector<ComputerCertificate> Certificates{};

public:
	MainDialog(HINSTANCE Instance);
	MainDialog(const MainDialog&) = delete;
	MainDialog& operator=(const MainDialog&) = delete;
	MainDialog(MainDialog&&) = delete;
	MainDialog& operator=(MainDialog&&) = delete;
	void Show(int ShowCommand) const noexcept { ShowWindow(DialogHandle, ShowCommand); }
	const HWND GetWindowHandle() const noexcept { return DialogHandle; }
};