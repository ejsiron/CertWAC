#include <Windows.h>
#include <wincrypt.h>
#include <iomanip>
#include <sstream>
#include "ComputerCertificate.h"
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "NCrypt.lib")

const char* ServerAuthenticationOID{ "1.3.6.1.5.5.7.3.1" };

class CertificateStore
{
private:
	HCERTSTORE Store;
public:
	CertificateStore(HCERTSTORE NewStore) :Store(NewStore) {}
	~CertificateStore() { if (Store) CertCloseStore(Store, 0); }
	HCERTSTORE operator()() noexcept { return Store; }
	const bool IsValid() const noexcept { return Store != nullptr; }
};

enum class CertNames
{
	Subject,
	Issuer
};

static std::wstring GetNameFromCertificate(PCCERT_CONTEXT pCertContext, CertNames DesiredCertName)
{
	PCERT_NAME_BLOB DesiredName;
	switch (DesiredCertName)
	{
	case CertNames::Issuer:
		DesiredName = &pCertContext->pCertInfo->Issuer;
		break;
	default:
		DesiredName = &pCertContext->pCertInfo->Subject;
		break;
	}

	DWORD NameSize{ 0 };
	std::wstring CertName{};
	NameSize = CertNameToStr(pCertContext->dwCertEncodingType, DesiredName,
		CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG, NULL, NameSize);
	if (NameSize)
	{
		CertName.resize(NameSize);
		CertNameToStr(pCertContext->dwCertEncodingType, DesiredName,
			CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG, CertName.data(), NameSize);
	}
	return CertName;
}

std::pair<ErrorRecord, std::vector<ComputerCertificate>> GetComputerCertificates()
{
	const wchar_t* StoreName{ L"MY" };
	std::vector<ComputerCertificate> Certificates{};

	auto ComputerStore = CertificateStore(
		CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0, NULL, CERT_SYSTEM_STORE_LOCAL_MACHINE, StoreName));

	if (!ComputerStore.IsValid())
	{
		return std::make_pair(ErrorRecord(GetLastError(), L"Opening computer certificate store"), Certificates);
	}

	PCCERT_CONTEXT pCertContext{ nullptr };
	while (pCertContext = CertEnumCertificatesInStore(ComputerStore(), pCertContext))
	{
		ComputerCertificate ThisCert{};
		ThisCert.SubjectName(GetNameFromCertificate(pCertContext, CertNames::Subject));
		ThisCert.Issuer(GetNameFromCertificate(pCertContext, CertNames::Issuer));
		ThisCert.ValidFrom(pCertContext->pCertInfo->NotBefore);
		ThisCert.ValidTo(pCertContext->pCertInfo->NotAfter);

		// Enhanced Key Usage
		DWORD EnhancedKeyUsageSize{ 0 };
		if (CertGetEnhancedKeyUsage(pCertContext, 0, NULL, &EnhancedKeyUsageSize))
		{
			std::vector<char> EKU{};
			EKU.resize(EnhancedKeyUsageSize);
			if (CertGetEnhancedKeyUsage(pCertContext, 0, (CERT_ENHKEY_USAGE*)EKU.data(), &EnhancedKeyUsageSize))
			{
				for (auto i{ 0 }; i != ((CERT_ENHKEY_USAGE*)(EKU.data()))->cUsageIdentifier; ++i)
				{
					if (std::string{ ServerAuthenticationOID } == std::string{ ((CERT_ENHKEY_USAGE*)(EKU.data()))->rgpszUsageIdentifier[i] })
					{
						ThisCert.HasServerAuthentication(true);
					}
				}

			}
		}

		// SAN
		PCERT_EXTENSION Extension = nullptr;
		PCERT_ALT_NAME_INFO pAlternateNameInfo = nullptr;
		PCERT_ALT_NAME_ENTRY pAlternateNameEntry = nullptr;

		std::string ExtensionOID{};
		std::vector<std::wstring> AlternateNames{};
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
					AlternateName.resize(DataSize);
					if (CryptFormatObject(pCertContext->dwCertEncodingType, 0, 0, NULL, szOID_SUBJECT_ALT_NAME2,
						Extension->Value.pbData, Extension->Value.cbData,
						(void*)AlternateName.data(), &DataSize))
					{
						AlternateNames.emplace_back(std::move(AlternateName));
					}
				}
			}
		}
		ThisCert.SubjectAlternateNames(AlternateNames);

		// thumbprint
		DWORD CertSHA1HashSize{ 0 };
		if (CryptHashCertificate2(BCRYPT_SHA1_ALGORITHM, 0, NULL, pCertContext->pbCertEncoded,
			pCertContext->cbCertEncoded, NULL, &CertSHA1HashSize))
		{
			std::vector<BYTE> ThumbprintBytes;
			ThumbprintBytes.resize(CertSHA1HashSize);
			if (CryptHashCertificate2(BCRYPT_SHA1_ALGORITHM, 0, NULL, pCertContext->pbCertEncoded,
				pCertContext->cbCertEncoded, ThumbprintBytes.data(), &CertSHA1HashSize))
			{
				std::wstringstream HashStream;
				HashStream << std::hex;
				for (auto const& HashByte : ThumbprintBytes)
				{
					HashStream << std::setw(2) << std::setfill(L'0') << HashByte;
				}
				ThisCert.Thumbprint(HashStream.str());
			}
		}
		
		// detect private key
		HCRYPTPROV_OR_NCRYPT_KEY_HANDLE PrivateKeyHandle;
		DWORD PrivateKeyInfo{ 0 };
		DWORD KeySpec{ 0 };
		BOOL MustFree{ FALSE };
		if (CryptAcquireCertificatePrivateKey(pCertContext, CRYPT_ACQUIRE_SILENT_FLAG | CRYPT_ACQUIRE_PREFER_NCRYPT_KEY_FLAG, NULL, &PrivateKeyHandle, &KeySpec, &MustFree))
		{
			ThisCert.HasPrivateKey(true);
		}
		if (MustFree)
		{
			if (KeySpec == CERT_NCRYPT_KEY_SPEC) { NCryptFreeObject(PrivateKeyHandle); }
			else { CryptReleaseContext(PrivateKeyHandle, 0); }
		}
		Certificates.emplace_back(std::move(ThisCert));
	}
	if (pCertContext != nullptr)
		CertFreeCertificateContext(pCertContext);

	return std::make_pair(ErrorRecord(ERROR_SUCCESS, L"Reading certificates"), Certificates);
}
