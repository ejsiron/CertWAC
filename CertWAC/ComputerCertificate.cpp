#include "ComputerCertificate.h"

static std::wstring GetStringFromFileTime(const FILETIME& Time) noexcept
{
	std::wstring ReturnTime{};
	SYSTEMTIME SystemTime{ 0 };
	FileTimeToSystemTime(&Time, &SystemTime);
	int TimeCharCount;
	if (TimeCharCount = GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, &SystemTime,
		NULL, NULL, 0, NULL))
	{
		ReturnTime.resize(TimeCharCount);
		GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, &SystemTime,
			NULL, ReturnTime.data(), TimeCharCount, NULL);
	}
	return ReturnTime;
}

const std::wstring ComputerCertificate::GetValidFrom() const noexcept
{
	return GetStringFromFileTime(ValidFrom);
}

const std::wstring ComputerCertificate::GetValidTo() const noexcept
{
	return GetStringFromFileTime(ValidTo);
}

const bool ComputerCertificate::IsWithinValidityPeriod() const noexcept
{
	SYSTEMTIME NowSystemTime{ 0 };
	GetSystemTime(&NowSystemTime);
	FILETIME NowFileTime{ 0 };
	SystemTimeToFileTime(&NowSystemTime, &NowFileTime);
	ULONGLONG qwNow{ (((ULONGLONG)ValidFrom.dwHighDateTime) << 32) + ValidFrom.dwLowDateTime };
	ULONGLONG qwValidFrom{ (((ULONGLONG)ValidFrom.dwHighDateTime) << 32) + ValidFrom.dwLowDateTime };
	ULONGLONG qwValidTo{ (((ULONGLONG)ValidTo.dwHighDateTime) << 32) + ValidTo.dwLowDateTime };
	return qwValidFrom < qwNow < qwValidTo;
}