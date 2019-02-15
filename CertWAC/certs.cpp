#include <Windows.h>
#include <Wincrypt.h>
#include <array>
#include <sstream>
#include "certs.h"
#pragma comment(lib, "crypt32.lib")

void GetComputerCertificates()
{
	HANDLE hStoreHandle{ 0 };
	PCCERT_CONTEXT pCertContext{ 0 };
	const wchar_t* StoreName{ L"MY" };

	if (hStoreHandle = CertOpenSystemStore(NULL, StoreName))
	{
		// this is good
	}
	else
	{
		// this is not, GetLastError has deets
	}
	
	DWORD PropID{ 0 };
	while (pCertContext = CertEnumCertificatesInStore(hStoreHandle, pCertContext))
	{
		DWORD CertHashSize{ 0 };
		if (CryptHashCertificate2(BCRYPT_SHA1_ALGORITHM, 0, NULL,
			pCertContext->pbCertEncoded, pCertContext->cbCertEncoded, NULL, &CertHashSize))
		{
			auto pCertHash{ new BYTE[CertHashSize] };
			SecureZeroMemory(pCertHash, sizeof(BYTE) * CertHashSize);
			// https://github.com/Microsoft/TSS.MSR/blob/master/PCPTool.v11/exe/SDKSample.cpp
			if (CryptHashCertificate2(BCRYPT_SHA1_ALGORITHM, 0, NULL,
				pCertContext->pbCertEncoded, pCertContext->cbCertEncoded, pCertHash, &CertHashSize))
			{
				std::stringstream CertHashStream;
				CertHashStream << std::hex;
				for (auto i{ 0 }; i != CertHashSize; ++i)
					CertHashStream << pCertHash[i];
				CertHashStream << L'\n';
				std::string CertHashString{ CertHashStream.str() };
				OutputDebugStringA(CertHashString.c_str());
			}
			delete[] pCertHash;
		}
	}

	if (!CertCloseStore(hStoreHandle, 0))
	{
		OutputDebugString(L"Could not close store\n");
	}
}
