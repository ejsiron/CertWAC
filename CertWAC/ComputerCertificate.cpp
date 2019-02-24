#include <regex>
#include "ComputerCertificate.h"
#include "StringUtility.h"

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
	return CleanWindowsString(ReturnTime);
}

void ComputerCertificate::SubjectName(const std::wstring& NewSubjectName)
{
	subjectname = std::move(CleanWindowsString(NewSubjectName));
}

void ComputerCertificate::Issuer(const std::wstring& NewIssuer)
{
	issuer = CleanWindowsString(NewIssuer);
}

void ComputerCertificate::Thumbprint(const std::wstring& NewThumbprint)
{
	thumbprint = CleanWindowsString(NewThumbprint);
}

const std::wstring ComputerCertificate::ValidFrom() const noexcept
{
	return GetStringFromFileTime(validfrom);
}

const std::wstring ComputerCertificate::ValidTo() const noexcept
{
	return GetStringFromFileTime(validto);
}

void ComputerCertificate::SubjectAlternateNames(const std::vector<std::wstring>& NewSubjectAlternateNames)
{
	subjectalternatenames = std::vector<std::wstring>{};
	for (auto const& NewSubjectAlternateName : NewSubjectAlternateNames)
	{
		subjectalternatenames.emplace_back(std::move(CleanWindowsString(NewSubjectAlternateName)));
	}
}

const bool ComputerCertificate::IsWithinValidityPeriod() const noexcept
{
	SYSTEMTIME NowSystemTime{ 0 };
	GetSystemTime(&NowSystemTime);
	FILETIME NowFileTime{ 0 };
	SystemTimeToFileTime(&NowSystemTime, &NowFileTime);
	ULONGLONG qwNow{ (((ULONGLONG)NowFileTime.dwHighDateTime) << 32) + NowFileTime.dwLowDateTime };
	ULONGLONG qwValidFrom{ (((ULONGLONG)validfrom.dwHighDateTime) << 32) + validfrom.dwLowDateTime };
	ULONGLONG qwValidTo{ (((ULONGLONG)validto.dwHighDateTime) << 32) + validto.dwLowDateTime };
	return (qwValidFrom < qwNow && qwNow < qwValidTo);
}

std::wstring ComputerCertificate::FQDNFromSimpleRDN(const std::wstring& RDN)
{
	std::vector<std::wstring>FQDNComponents{};
	const std::wregex FQDNComponentMatch{ L"(?:DC|CN)=([^,]*)" };
	using rxit = std::wsregex_iterator;
	rxit EmptyRegexIterator{};
	for (rxit DomainPartWalker(RDN.cbegin(), RDN.cend(), FQDNComponentMatch);
		DomainPartWalker != EmptyRegexIterator; ++DomainPartWalker)
	{
		FQDNComponents.emplace_back((*DomainPartWalker)[1]);
	}
	return JoinString(L".", FQDNComponents);
}