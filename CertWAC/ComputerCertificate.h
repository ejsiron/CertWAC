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
	std::vector<std::wstring> subjectalternatenames{};
	bool serverauthentication;
	bool privatekey;

public:
	ComputerCertificate() {}
	ComputerCertificate(const ComputerCertificate&) = default;
	ComputerCertificate& operator=(const ComputerCertificate&) = default;
	ComputerCertificate(ComputerCertificate&&)=default;
	ComputerCertificate& operator=(ComputerCertificate&&) = default;
	void SubjectName(const std::wstring& NewSubjectName);
	const std::wstring& SubjectName() const noexcept { return subjectname; }
	void Issuer(const std::wstring& NewIssuer);
	const std::wstring Issuer() const noexcept { return issuer; }
	void Thumbprint(const std::wstring& NewThumbprint);
	const std::wstring Thumbprint() const noexcept { return thumbprint; }
	void ValidFrom(const FILETIME NewValidityStart) noexcept { validfrom = NewValidityStart; }
	const std::wstring ValidFrom() const noexcept;
	void ValidTo(const FILETIME NewValidityEnd) noexcept { validto = NewValidityEnd; }
	const std::wstring ValidTo() const noexcept;
	void SubjectAlternateNames(const std::vector<std::wstring>& NewSubjectAlternateNames);
	const std::vector<std::wstring>& SubjectAlternateNames() const noexcept { return subjectalternatenames; }
	void HasServerAuthentication(const bool HasServerAuthenticationEKU) noexcept { serverauthentication = HasServerAuthenticationEKU; }
	const bool HasServerAuthentication() const noexcept { return serverauthentication; }
	void HasPrivateKey(const bool PrivateKeyExists) { privatekey = PrivateKeyExists; }
	const bool HasPrivateKey() const noexcept { return privatekey; }
	const bool IsWithinValidityPeriod() const noexcept;
	static std::wstring FQDNFromSimpleRDN(const std::wstring& RDN);
};

std::pair<ErrorRecord, std::vector<ComputerCertificate>> GetComputerCertificates();