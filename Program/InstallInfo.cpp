#include <Windows.h>
#include <optional>
#include <memory>
#include "InstallInfo.h"

const wchar_t* RegistryExpectedAppRoot{ L"SOFTWARE\\Microsoft\\ServerManagementGateway" };
const wchar_t* PortFieldName{ L"SmePort" };
const wchar_t* RegistryUninstallRoot{ L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall" };
const wchar_t* ExpectedApplicationName{ L"Windows Admin Center" };
const wchar_t* AppDisplayNameFieldName{ L"DisplayName" };
const wchar_t* AppModifyPathFieldName{ L"ModifyPath" };

const wchar_t* ActivityAccessingUninstallRegistry{ L"Accessing uninstall registry branch" };
const wchar_t* ActivitySearchingForInstallation{ L"Searching for WAC installation key" };
const wchar_t* ActivityReadingAppModifyKVP{ L"Reading app modify command line" };
const wchar_t* ActivityAccessingAppRegistry{ L"Accessing application registry branch" };
const wchar_t* ActivityReadingPortKVP{ L"Reading port value from registry" };
const wchar_t* ActivitySettingPortKVP{ L"Writing port value to registry" };

constexpr DWORD REG_READ_ACCESS{ KEY_READ | KEY_WOW64_64KEY };
constexpr DWORD KVPValueLength{ 1024 };

auto DeleteHKEY = [](HKEY Key) noexcept { RegCloseKey(Key); };
using RegistryKey = std::unique_ptr<HKEY__, decltype(DeleteHKEY)>;

static std::pair<LSTATUS, RegistryKey> OpenRegistryKey(const HKEY StartKey, const std::wstring& SubkeyName, const bool WriteAccess = false) noexcept
{
	HKEY DesiredKey{ 0 };
	LSTATUS Result{ RegOpenKeyEx(StartKey, SubkeyName.c_str(), 0, REG_READ_ACCESS | (WriteAccess ? KEY_WRITE : 0), &DesiredKey) };
	return std::make_pair(Result, RegistryKey(DesiredKey, DeleteHKEY));
}

static std::pair<LSTATUS, DWORD> GetMaxSubkeyNameLength(HKEY ParentKey) noexcept
{
	DWORD MaxSubkeySize{ 0 };
	LSTATUS SizeQueryResult{ RegQueryInfoKey(ParentKey, NULL, NULL, NULL, NULL, &MaxSubkeySize, NULL, NULL, NULL, NULL, NULL, NULL) };
	return std::make_pair(SizeQueryResult, MaxSubkeySize);
}

static std::pair<LSTATUS, std::wstring> GetKVPStringValue(HKEY ParentKey, const wchar_t* KVPKeyName) noexcept
{
	DWORD ValueType;
	DWORD RetrievedValueLength{ KVPValueLength };
	wchar_t ReceiveBuffer[KVPValueLength];
	SecureZeroMemory(ReceiveBuffer, KVPValueLength);
	LSTATUS ValueQueryResult{ RegQueryValueEx(ParentKey, KVPKeyName, NULL, &ValueType, (LPBYTE)ReceiveBuffer, &RetrievedValueLength) };
	if (ValueQueryResult == ERROR_SUCCESS && !((ValueType == REG_SZ) || (ValueType == REG_EXPAND_SZ)))
	{
		ValueQueryResult = ERROR_FILE_NOT_FOUND;
		ReceiveBuffer[0] = 0;
	}
	return std::make_pair(ValueQueryResult, std::wstring{ ReceiveBuffer });
}

static std::pair<LSTATUS, std::wstring> FindSubkeyWithExpectedKVP(HKEY ParentKey, const wchar_t* KVPKeyName, const wchar_t* KVPKeyValue) noexcept
{
	std::wstring SubkeyName{};
	auto[GetMaxSubkeyLengthResult, SubkeyNameLength] { GetMaxSubkeyNameLength(ParentKey)};
	if (GetMaxSubkeyLengthResult == ERROR_SUCCESS)
	{
		std::wstring RetrievedKeyName{};
		RetrievedKeyName.reserve(++SubkeyNameLength);
		LSTATUS SubEnumResult{ 0 };
		DWORD SubkeyIndex{ 0 };
		DWORD SubkeyNameRetrievedSize{ SubkeyNameLength };
		while ((SubEnumResult = RegEnumKeyEx(ParentKey, SubkeyIndex, RetrievedKeyName.data(), &SubkeyNameRetrievedSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
		{
			++SubkeyIndex;
			SubkeyNameRetrievedSize = SubkeyNameLength;	// reset during each iteration or the next will fail
			auto[SubkeyOpenResult, Subkey] { OpenRegistryKey(ParentKey, RetrievedKeyName)};
			if (SubkeyOpenResult != ERROR_SUCCESS)
				return std::make_pair(SubkeyOpenResult, SubkeyName);

			auto[GetValueResult, Value] { GetKVPStringValue(Subkey.get(), KVPKeyName)};
			if (GetValueResult == ERROR_SUCCESS && Value == KVPKeyValue)
			{
				return std::make_pair(GetValueResult, RetrievedKeyName);
			}
		}
		if (SubEnumResult == ERROR_NO_MORE_ITEMS)
			GetMaxSubkeyLengthResult = ERROR_FILE_NOT_FOUND;
		else
			GetMaxSubkeyLengthResult = SubEnumResult;
	}
	return std::make_pair(GetMaxSubkeyLengthResult, SubkeyName);
}

static std::pair<ErrorRecord, std::wstring> GetModifyPath() noexcept
{
	std::wstring MSICmd{};
	auto[RootOpenResult, RootKey] { OpenRegistryKey(HKEY_LOCAL_MACHINE, RegistryUninstallRoot)};
	if (RootOpenResult != ERROR_SUCCESS)
	{
		return std::pair(ErrorRecord(RootOpenResult, ActivityAccessingUninstallRegistry), MSICmd);
	}

	auto[InstallOpenResult, OptInstallKeyName] { FindSubkeyWithExpectedKVP(RootKey.get(),
		AppDisplayNameFieldName, ExpectedApplicationName)};
	LSTATUS ActivityResult{ ERROR_SUCCESS };
	if (InstallOpenResult == ERROR_SUCCESS)
	{
		auto[OptOpenResult, OptInstallKey] {OpenRegistryKey(RootKey.get(), OptInstallKeyName)};
		ActivityResult = OptOpenResult;
		if (ActivityResult == ERROR_SUCCESS)
		{
			auto[GetPathResult, DiscoveredPath] { GetKVPStringValue(OptInstallKey.get(), AppModifyPathFieldName)};
			ActivityResult = GetPathResult;
			MSICmd = DiscoveredPath;
		}
	}
	return std::pair(ErrorRecord(ActivityResult, ActivitySearchingForInstallation), MSICmd);
}

static std::pair<ErrorRecord, int> GetListeningPort() noexcept
{
	int Port{ 0 };
	auto[RootOpenResult, RootKey] { OpenRegistryKey(HKEY_LOCAL_MACHINE, RegistryExpectedAppRoot)};
	if (RootOpenResult != ERROR_SUCCESS)
	{
		return std::pair(ErrorRecord(RootOpenResult, ActivityAccessingAppRegistry), Port);
	}
	auto[GetPortResult, DiscoveredPort] { GetKVPStringValue(RootKey.get(), PortFieldName)};
	if (GetPortResult == ERROR_SUCCESS)
		Port = std::stoi(std::wstring{ DiscoveredPort });
	return std::pair(ErrorRecord(GetPortResult, ActivityReadingPortKVP), Port);
}

std::tuple<ErrorRecord, std::wstring, int> GetWACInstallInfo() noexcept
{
	auto [GetModifyRegistryResult, ModifyPath] {GetModifyPath()};
	if (GetModifyRegistryResult.ErrorCode() == ERROR_SUCCESS)
	{
		auto [GetPortResult, Port] {GetListeningPort()};
		return std::make_tuple(GetPortResult, ModifyPath, Port);
	}
	return std::make_tuple(GetModifyRegistryResult, std::wstring{}, 0);
}

ErrorRecord SetListeningPort(const unsigned int Port) noexcept
{
	auto[RootOpenResult, RootKey] { OpenRegistryKey(HKEY_LOCAL_MACHINE, RegistryExpectedAppRoot, true)};
	if (RootOpenResult != ERROR_SUCCESS)
	{
		return ErrorRecord(RootOpenResult, ActivityAccessingAppRegistry);
	}
	auto PortBytes{ std::to_wstring(Port) };
	return ErrorRecord(RegSetValueEx(
		RootKey.get(), PortFieldName, 0, REG_SZ, (BYTE*)PortBytes.c_str(),
		(PortBytes.size() + 1) * 2),	// wants bytes, these are wchar_ts
		ActivitySettingPortKVP);
}
