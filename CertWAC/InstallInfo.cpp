#include <Windows.h>
#include <winerror.h>
#include <optional>
#include <sstream>
#include <memory>
#include "InstallInfo.h"

const wchar_t* RegistryUninstallRoot{ L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall" };
const wchar_t* ExpectedWACGUID{ L"{4FAE3A2E-4369-490E-97F3-0B3BFF183AB9}" };
const wchar_t* ExpectedApplicationName{ L"Windows Admin Center" };
const wchar_t* AppDisplayNameFieldName{ L"DisplayName" };
const wchar_t* AppModifyPathFieldName{ L"ModifyPath" };
constexpr DWORD REG_ACCESS{ KEY_READ | KEY_WOW64_64KEY };
constexpr DWORD KVPValueLength{ 1024 };

auto DeleteHKEY = [](HKEY Key) { RegCloseKey(Key); };

using RegistryKey = std::unique_ptr<HKEY__, decltype(DeleteHKEY)>;

static std::pair<LSTATUS, RegistryKey> OpenRegistryKey(HKEY StartKey, const std::wstring& SubkeyName)
{
	HKEY DesiredKey{ 0 };
	LSTATUS Result{ RegOpenKeyEx(StartKey, SubkeyName.c_str(), 0, REG_ACCESS, &DesiredKey) };
	return std::make_pair(Result, RegistryKey(DesiredKey, DeleteHKEY));
}

static std::pair<LSTATUS, DWORD> GetMaxSubkeyNameLength(HKEY ParentKey)
{
	DWORD MaxSubkeySize{ 0 };
	LSTATUS SizeQueryResult{ RegQueryInfoKey(ParentKey, NULL, NULL, NULL, NULL, &MaxSubkeySize, NULL, NULL, NULL, NULL, NULL, NULL) };
	return std::make_pair(SizeQueryResult, MaxSubkeySize);
}

static std::pair<LSTATUS, std::wstring> GetKVPStringValue(HKEY ParentKey, const wchar_t* KVPKeyName)
{
	DWORD ValueType;
	DWORD RetrievedValueLength{ KVPValueLength };
	wchar_t ReceiveBuffer[KVPValueLength];
	SecureZeroMemory(ReceiveBuffer, KVPValueLength);
	LSTATUS ValueQueryResult{ RegQueryValueEx(ParentKey, KVPKeyName, NULL, &ValueType, (LPBYTE)ReceiveBuffer, &RetrievedValueLength) };
	if (ValueQueryResult == ERROR_SUCCESS && !((ValueType == REG_SZ) || (ValueType == REG_EXPAND_SZ)))
	{
		ValueQueryResult = ERROR_FILE_NOT_FOUND;
		ReceiveBuffer[0] = L'\0';
	}
	return std::make_pair(ValueQueryResult, std::wstring{ ReceiveBuffer });
}

static std::pair<LSTATUS, std::optional<RegistryKey>> FindSubkeyWithExpectedKVP(HKEY ParentKey, const wchar_t* KVPKeyName, const wchar_t* KVPKeyValue)
{
	auto[ActionResult, SubkeyNameLength] = GetMaxSubkeyNameLength(ParentKey);
	if (ActionResult == ERROR_SUCCESS)
	{
		std::wstring RetrievedKeyName{};
		SubkeyNameLength++;
		RetrievedKeyName.reserve(SubkeyNameLength);
		LSTATUS SubEnumResult{ 0 };
		DWORD SubkeyIndex{ 0 };
		DWORD SubkeyNameRetrievedSize{ SubkeyNameLength };
		while ((SubEnumResult = RegEnumKeyEx(ParentKey, SubkeyIndex, RetrievedKeyName.data(), &SubkeyNameRetrievedSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
		{
			SubkeyIndex++;
			SubkeyNameRetrievedSize = SubkeyNameLength;	// reset during each iteration or the next will fail
			auto[SubkeyOpenResult, Subkey] = OpenRegistryKey(ParentKey, RetrievedKeyName);
			if (SubkeyOpenResult)
				return std::make_pair(SubkeyOpenResult, std::nullopt);
				
			auto[GetValueResult, Value] = GetKVPStringValue(Subkey.get(), KVPKeyName);
			if (GetValueResult == ERROR_SUCCESS && Value == KVPKeyValue)
			{
				return std::make_pair(GetValueResult, std::move(Subkey));
			}
		}
		if (SubEnumResult == ERROR_NO_MORE_ITEMS)
			ActionResult = ERROR_FILE_NOT_FOUND;
		else
			ActionResult = SubEnumResult;
	}
	return std::make_pair(ActionResult, std::nullopt);
}

std::pair<ErrorRecord, std::wstring> GetMSIModifyPath()
{
	std::wstring MSICmd{};
	HKEY RawInstallKey{ 0 };

	auto[RootOpenResult, RootKey] = OpenRegistryKey(HKEY_LOCAL_MACHINE, RegistryUninstallRoot);
	if (RootOpenResult != ERROR_SUCCESS)
	{
		return std::make_pair(ErrorRecord(RootOpenResult, L"Accessing uninstall registry branch"), MSICmd);
	}

	auto[InstallOpenResult, ExpectedInstallKey] = OpenRegistryKey(RootKey.get(), ExpectedWACGUID);
	if (InstallOpenResult == ERROR_FILE_NOT_FOUND)
	{
		auto[InstallOpenResult, OptInstallKey] = FindSubkeyWithExpectedKVP(RootKey.get(),
			AppDisplayNameFieldName, ExpectedApplicationName);
		if (InstallOpenResult == ERROR_SUCCESS && OptInstallKey)
		{
			RawInstallKey = OptInstallKey.value().get();
		}
		else
		{
			return std::make_pair(ErrorRecord(InstallOpenResult, L"Searching for WAC installation"), MSICmd);
		}
	}
	else if (InstallOpenResult != ERROR_SUCCESS)
	{
		return std::make_pair(ErrorRecord(InstallOpenResult, L"Searching for expected installation key"), MSICmd);
	}
	else
	{
		RawInstallKey = ExpectedInstallKey.get();
	}
	
	auto[GetPathResult, DiscoveredPath] = GetKVPStringValue(RawInstallKey, AppModifyPathFieldName);
	return std::make_pair(ErrorRecord(GetPathResult, L"Reading WAC install path key"), DiscoveredPath);
}
