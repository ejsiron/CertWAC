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
			// Subject Name
			DWORD NameSize{ 0 };
			NameSize = CertNameToStr(pCertContext->dwCertEncodingType, &pCertContext->pCertInfo->Subject,
				CERT_OID_NAME_STR, NULL, NameSize);
			if (NameSize)
			{
				auto RawName{ new wchar_t[NameSize] };
				CertNameToStr(pCertContext->dwCertEncodingType, &pCertContext->pCertInfo->Subject,
					CERT_OID_NAME_STR, RawName, NameSize);
				OutputDebugString(RawName);
				OutputDebugString(L"\r\n");
				delete[] RawName;
			}

			// Issuer
			NameSize = 0;
			NameSize = CertNameToStr(pCertContext->dwCertEncodingType, &pCertContext->pCertInfo->Issuer,
				CERT_OID_NAME_STR, NULL, NameSize);
			if (NameSize)
			{
				auto RawName{ new wchar_t[NameSize] };
				CertNameToStr(pCertContext->dwCertEncodingType, &pCertContext->pCertInfo->Issuer,
					CERT_OID_NAME_STR, RawName, NameSize);
				OutputDebugString(RawName);
				OutputDebugString(L"\r\n");
				delete[] RawName;
			}

			// Start
			DWORD PropertyID{ 0 };
			DWORD cbData{ 0 };
			std::stringstream ss;
			while (PropertyID = CertEnumCertificateContextProperties(pCertContext, PropertyID))
			{
				//if (CertGetCertificateContextProperty(pCertContext, PropertyID, NULL, &cbData))
				//{
				//	ss.clear();
				//	ss << "Property: " << PropertyID << "\r\n";
				//	std::string debugstring{ ss.str() };
				//	OutputDebugStringA(debugstring.c_str());
				//	switch (PropertyID)
				//	{
				//	case(CERT_ENHKEY_USAGE_PROP_ID):
				//		auto RawEnhancedKeyUsage{ new BYTE[cbData] };
				//		if (CertGetCertificateContextProperty(pCertContext, PropertyID, RawEnhancedKeyUsage, &cbData))
				//		{
				//			
				//			for (auto i{ 0 }; i != ((CTL_USAGE*)RawEnhancedKeyUsage)->cUsageIdentifier; ++i)
				//			{
				//				OutputDebugStringA(((CTL_USAGE*)RawEnhancedKeyUsage)->rgpszUsageIdentifier[i]);
				//			}
				//		}
				//		break;
				//	}
				//}//else broken
			}

			//BOOL CertGetEnhancedKeyUsage(
			//	PCCERT_CONTEXT     pCertContext,
			//	DWORD              dwFlags,
			//	PCERT_ENHKEY_USAGE pUsage,
			//	DWORD              *pcbUsage
			//);

			// Enhanced Key Usage
			DWORD EnhancedKeyUsageSize{ 0 };
			if (CertGetEnhancedKeyUsage(pCertContext, 0, NULL, &EnhancedKeyUsageSize))
			{
				PCERT_ENHKEY_USAGE pKeyEnhancedKeyUsage{ (CERT_ENHKEY_USAGE*)(new char[EnhancedKeyUsageSize]) };
				if (CertGetEnhancedKeyUsage(pCertContext, 0, pKeyEnhancedKeyUsage, &EnhancedKeyUsageSize))
				{
					OutputDebugString(L"Well at least that worked\r\n");
					
				}
				delete[] pKeyEnhancedKeyUsage;
			}

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
