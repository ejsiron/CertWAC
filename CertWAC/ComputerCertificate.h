#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include "ErrorRecord.h"

constexpr size_t SHA1_BYTE_LENGTH{ 20 };

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
	void EKUServerAuthentication(const bool HasServerAuthenticationEKU) noexcept { serverauthentication = HasServerAuthenticationEKU; }
	const bool EKUServerAuthentication() const noexcept { return serverauthentication; }
	void HasPrivateKey(const bool PrivateKeyExists) { privatekey = PrivateKeyExists; }
	const bool HasPrivateKey() const noexcept { return privatekey; }
	const bool IsWithinValidityPeriod() const noexcept;
	static std::wstring FQDNFromSimpleRDN(const std::wstring& RDN);
};

std::pair<ErrorRecord, std::vector<ComputerCertificate>> GetComputerCertificates() noexcept;