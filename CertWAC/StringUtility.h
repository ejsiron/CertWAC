#pragma once
#include <string>
#include <vector>

// use with std::wstring created from Windows APIs that write a terminating null zero
// -- does not remove nulls from middle of a string (ex: registry REG_MULTI_SZ)
std::wstring CleanWindowsString(const std::wstring& WindowsString);

// Combines a vector of strings into a single string, separated by Glue
std::wstring JoinString(const std::wstring& Glue, const std::vector<std::wstring>& Parts);
template <class T>
std::wstring JoinString(const T Glue, const std::vector<std::wstring>& Parts) { return JoinString(std::wstring{ Glue }, Parts); }

// Splits a string, using a regex match against GluePattern
// * only YOU can prevent ill-formed regex patterns
std::vector<std::wstring> SplitString(const std::wstring& GluePattern, const std::wstring& Composite);
// Splits a string, using a regex match against GluePattern
// * only YOU can prevent ill-formed regex patterns
template <class T>
std::vector<std::wstring> SplitString(const T GluePattern, const std::wstring& Composite) { return SplitString(std::wstring(GluePattern), Composite); }