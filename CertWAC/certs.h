#pragma once
#include <Windows.h>
#include <array>
#include <string>

constexpr size_t SHA1_BYTE_LENGTH{ 20 };
void GetComputerCertificates();
class StoreCertificates
{
private:
	std::wstring SubjectName{};
	std::wstring Issuer{};
	std::array<BYTE, SHA1_BYTE_LENGTH> Thumbprint{0};
	FILETIME ValidFrom;
	FILETIME ValidTo;
};