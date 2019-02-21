#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "ErrorRecord.h"

constexpr size_t SHA1_BYTE_LENGTH{ 20 };

class ComputerCertificate
{
private:
	std::wstring SubjectName{};
	std::wstring Issuer{};
	std::wstring Thumbprint{};
	FILETIME ValidFrom{ 0 };
	FILETIME ValidTo{ 0 };
	std::vector<std::wstring> SubjectAlternateNames{};
	bool ServerAuthentication;
	bool PrivateKey;
	friend std::pair<ErrorRecord, std::vector<ComputerCertificate>> GetComputerCertificates() noexcept;

public:
	ComputerCertificate() {}
	ComputerCertificate(const ComputerCertificate&) = default;
	ComputerCertificate& operator=(const ComputerCertificate&) = default;
	ComputerCertificate(ComputerCertificate&&)=default;
	ComputerCertificate& operator=(ComputerCertificate&&) = default;
	const std::wstring& GetSubjectName() const noexcept { return SubjectName; }
	const std::wstring& GetIssuer() const noexcept { return Issuer; }
	const std::wstring GetThumbprint() const noexcept { return Thumbprint; }
	const std::wstring GetValidFrom() const noexcept;
	const std::wstring GetValidTo() const noexcept;
	const std::vector<std::wstring>& GetSubjectAlternateNames() const noexcept { return SubjectAlternateNames; }
	const bool EKUServerAuthentication() const noexcept { return ServerAuthentication; }
	const bool HasPrivateKey() const noexcept { return PrivateKey; }
	const bool IsWithinValidityPeriod() const noexcept;
};

std::pair<ErrorRecord, std::vector<ComputerCertificate>> GetComputerCertificates() noexcept;