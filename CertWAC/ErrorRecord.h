#pragma once
#include <Windows.h>
#include <string>

class ErrorRecord
{
private:
	DWORD errorcode;
	std::wstring activity;
public:
	ErrorRecord(DWORD Code = 0, std::wstring ActivityText = L"Unknown") : errorcode(Code), activity(std::move(ActivityText)) {}
	ErrorRecord(const ErrorRecord&) = default;
	ErrorRecord& operator=(const ErrorRecord&) = default;
	ErrorRecord(ErrorRecord&&) = default;
	ErrorRecord& operator=(ErrorRecord&&) = default;
	const DWORD ErrorCode() const noexcept { return errorcode; }
	static const std::wstring TranslateErrorCode(DWORD ErrorCode);
	const std::wstring ErrorMessage() const noexcept { return TranslateErrorCode(errorcode); }
	const std::wstring Activity() const noexcept { return activity; }
};
