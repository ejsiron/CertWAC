#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "ErrorRecord.h"

// represents a PKI certificate from the local Windows certificate store
// only exposes portions germane to this application
class ComputerCertificate
{
private:
	std::wstring subjectname{};
	std::wstring issuer{};
	std::wstring thumbprint{};
	FILETIME validfrom{ 0 };
	FILETIME validto{ 0 };
	std::wstring subjectalternatenames{};
	bool serverauthentication{ false };
	bool privatekey{ false };
	friend std::pair<ErrorRecord, std::vector<ComputerCertificate>> GetComputerCertificates();

public:
	ComputerCertificate() {}
	ComputerCertificate(const ComputerCertificate&) = default;
	ComputerCertificate& operator=(const ComputerCertificate&) = default;
	ComputerCertificate(ComputerCertificate&&)=default;
	ComputerCertificate& operator=(ComputerCertificate&&) = default;
	const std::wstring& SubjectName() const { return subjectname; }
	const std::wstring Issuer() const { return issuer; }
	const std::wstring Thumbprint() const { return thumbprint; }
	const std::wstring ValidFrom() const;
	const std::wstring ValidTo() const;
	std::wstring SubjectAlternateNames() const noexcept { return subjectalternatenames; }
	const bool HasServerAuthentication() const noexcept { return serverauthentication; }
	const bool HasPrivateKey() const noexcept { return privatekey; }
	const bool IsWithinValidityPeriod() const noexcept;
	static std::wstring FQDNFromSimpleRDN(const std::wstring& RDN);
};

std::pair<ErrorRecord, std::vector<ComputerCertificate>> GetComputerCertificates();