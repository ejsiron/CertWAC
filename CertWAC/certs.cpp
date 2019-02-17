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

			auto Start = pCertContext->pCertInfo->NotBefore;
			auto Expire = pCertContext->pCertInfo->NotAfter;

			// Enhanced Key Usage
			DWORD EnhancedKeyUsageSize{ 0 };
			if (CertGetEnhancedKeyUsage(pCertContext, 0, NULL, &EnhancedKeyUsageSize))
			{
				PCERT_ENHKEY_USAGE pKeyEnhancedKeyUsage{ (CERT_ENHKEY_USAGE*)(new char[EnhancedKeyUsageSize]) };
				if (CertGetEnhancedKeyUsage(pCertContext, 0, pKeyEnhancedKeyUsage, &EnhancedKeyUsageSize))
				{
					for (auto i{ 0 }; i != pKeyEnhancedKeyUsage->cUsageIdentifier; ++i)
					{
						OutputDebugString(L"EKU: ");
						OutputDebugStringA(pKeyEnhancedKeyUsage->rgpszUsageIdentifier[i]);
						OutputDebugString(L"\r\n");
					}

				}
				delete[] pKeyEnhancedKeyUsage;
			}

			// SAN
			PCERT_EXTENSION Extension = nullptr;
			PCERT_ALT_NAME_INFO pAlternateNameInfo = nullptr;
			PCERT_ALT_NAME_ENTRY pAlternateNameEntry = nullptr;
			DWORD dwAlternateNameInfoSize;

			std::string ExtensionOID{};
			for (auto i{ 0 }; i != pCertContext->pCertInfo->cExtension; ++i)
			{
				Extension = &pCertContext->pCertInfo->rgExtension[i];
				ExtensionOID.assign(Extension->pszObjId);
				if (ExtensionOID == szOID_SUBJECT_ALT_NAME || ExtensionOID == szOID_SUBJECT_ALT_NAME2)
				{
					DWORD DataSize{ 0 };
					if (CryptFormatObject(pCertContext->dwCertEncodingType, 0, 0, NULL, szOID_SUBJECT_ALT_NAME2,
						Extension->Value.pbData, Extension->Value.cbData, NULL, &DataSize))
					{
						std::wstring AlternateName{};
						AlternateName.reserve(DataSize);
						if (CryptFormatObject(pCertContext->dwCertEncodingType, 0, 0, NULL, szOID_SUBJECT_ALT_NAME2,
							Extension->Value.pbData, Extension->Value.cbData,
							(void*)AlternateName.data(), &DataSize))
						{
							OutputDebugString(L"Data: ");
							OutputDebugString(AlternateName.c_str());
							OutputDebugString(L"\r\n");
						}
					}
				}
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
