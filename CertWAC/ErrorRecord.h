#pragma once
#include <Windows.h>
#include <string>

class ErrorRecord
{
private:
	DWORD ErrorCode;
	std::wstring Activity;
public:
	ErrorRecord(DWORD LastError = 0, std::wstring LastActivity = L"Unknown") : ErrorCode(LastError), Activity(std::move(LastActivity)) {}
	ErrorRecord(const ErrorRecord&) = default;
	ErrorRecord& operator=(const ErrorRecord&) = default;
	ErrorRecord(ErrorRecord&&) = default;
	ErrorRecord& operator=(ErrorRecord&&) = default;
	const DWORD GetErrorCode() const noexcept { return ErrorCode; }
	static const std::wstring GetErrorMessage(DWORD ErrorCode) noexcept;
	const std::wstring GetErrorMessage() const noexcept { return GetErrorMessage(ErrorCode); }
	const std::wstring GetActivity() const noexcept { return Activity; }
};