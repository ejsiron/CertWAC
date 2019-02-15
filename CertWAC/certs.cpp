#include <Windows.h>
#include <wincrypt.h>
#include <iomanip>
#include <sstream>
#include "certs.h"
#pragma comment(lib, "crypt32.lib")

void GetComputerCertificates()
{
	PCCERT_CONTEXT pCertContext{ 0 };
	const wchar_t* StoreName{ L"MY" };
	HCERTSTORE CertificateStore{ NULL };

	if (CertificateStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0, NULL,
		CERT_SYSTEM_STORE_LOCAL_MACHINE, StoreName))
	{
		// this is good
	}
	else
	{
		// this is not, GetLastError has deets
	}

	DWORD PropID{ 0 };
	while (pCertContext = CertEnumCertificatesInStore(CertificateStore, pCertContext))
	{
		DWORD CertHashSize{ 0 };
		if (CryptHashCertificate2(BCRYPT_SHA1_ALGORITHM, 0, NULL,
			pCertContext->pbCertEncoded, pCertContext->cbCertEncoded, NULL, &CertHashSize))
		{
			// thumbprint
			auto pCertHash{ new BYTE[CertHashSize] };
			SecureZeroMemory(pCertHash, sizeof(BYTE) * CertHashSize);
			// https://github.com/Microsoft/TSS.MSR/blob/master/PCPTool.v11/exe/SDKSample.cpp
			if (CryptHashCertificate2(BCRYPT_SHA1_ALGORITHM, 0, NULL,
				pCertContext->pbCertEncoded, pCertContext->cbCertEncoded, pCertHash, &CertHashSize))
			{
				std::stringstream CertHashStream;
				CertHashStream << std::hex;
				for (auto i{ 0 }; i != CertHashSize; ++i)
					CertHashStream << std::setw(2) << std::setfill('0') << static_cast<int>(pCertHash[i]);
				CertHashStream << " END \r\n";
				std::string CertHashString{ CertHashStream.str() };
				OutputDebugStringA(CertHashString.c_str());
			}
			delete[] pCertHash;

			
		}
	}

	if (!CertFreeCertificateContext(pCertContext))
	{
		OutputDebugString(L"Could not free certificate\n");
	}

	if (!CertCloseStore(CertificateStore, 0))
	{
		OutputDebugString(L"Could not close store\n");
	}
}
