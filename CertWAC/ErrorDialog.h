#pragma once

#include <Windows.h>
#include <string>
#include "ErrorRecord.h"
#include "resource.h"

class ErrorDialog
{
private:
	/* Windows components */
	const HINSTANCE AppInstance{ 0 };
	HWND HandleDialogError{ 0 };
	static INT_PTR CALLBACK SharedDialogProc(HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam);
	INT_PTR CALLBACK ThisDialogProc(UINT uMessage, WPARAM wParam, LPARAM lParam);

	/* Controller */
	const ErrorRecord Error;

public:
	ErrorDialog(const HINSTANCE Instance, const HWND Parent, const ErrorRecord& ErrorDetails);
	ErrorDialog(const ErrorDialog&) = delete;
	ErrorDialog& operator=(const ErrorDialog&) = delete;
	ErrorDialog(ErrorDialog&&) = delete;
	ErrorDialog& operator=(ErrorDialog&&) = delete;
};