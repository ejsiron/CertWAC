#pragma once
#include <string>
#include <vector>

// use with std::wstring created from Windows APIs that write a terminating null zero
// -- does not remove nulls from middle of a string (ex: registry REG_MULTI_SZ)
std::wstring CleanWindowsString(const std::wstring& WindowsString);

std::wstring JoinString(const std::wstring& Glue, const std::vector<std::wstring>& Parts);
template <class T>
std::wstring JoinString(const T Glue, const std::vector<std::wstring>& Parts) { return JoinString(std::wstring{ Glue }, Parts); }

std::vector<std::wstring> SplitString(const std::wstring& Glue, const std::wstring& Composite);
template <class T>
std::wstring JoinString(const T Glue, const std::wstring& Composite) { return SplitString(std::wstring(Glue), Composite); }