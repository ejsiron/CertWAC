#pragma once
#include <Windows.h>
#include <array>

constexpr size_t SHA1_BYTE_LENGTH{ 20 };
void GetComputerCertificates();
class StoreCertificates
{
private:
	std::array<BYTE, SHA1_BYTE_LENGTH> Thumbprint{0};

};