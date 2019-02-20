#include "ErrorRecord.h"

const std::wstring ErrorRecord::GetErrorMessage() const noexcept
{
	std::wstring ErrorMessage{};
	LPTSTR szLocalErrorBuffer{ nullptr };
	size_t nMessageLengthChars{ 0 };
	size_t nMessageLengthBytes{ 0 };

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, ErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&szLocalErrorBuffer, 0, NULL
	);
	if (szLocalErrorBuffer)
	{
		ErrorMessage.assign(szLocalErrorBuffer);
		LocalFree(szLocalErrorBuffer);
	}
	else
	{
		ErrorMessage.assign(L"Unable to process error message.");
	}
	return ErrorMessage;
}
