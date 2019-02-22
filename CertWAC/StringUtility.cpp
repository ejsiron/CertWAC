#include <algorithm>
#include <regex>
#include "StringUtility.h"

std::wstring CleanWindowsString(const std::wstring& WindowsString)
{
	std::wstring NewString{ WindowsString };
	while (NewString.size() && *NewString.crbegin() == L'\0')
	{
		NewString.pop_back();
	}
	return NewString;
}

std::wstring JoinString(const std::wstring& Glue, const std::vector<std::wstring>& Parts)
{
	std::wstring JoinedString{};
	if (Parts.size())
	{
		JoinedString = *Parts.cbegin();
		if (JoinedString.size() > 1)
		{
			for (auto Part{ ++Parts.cbegin() }; Part != Parts.cend(); ++Part)
			{
				JoinedString += Glue + *Part;
			}
		}
	}
	return JoinedString;
}

static std::wstring SanitizeRegexPattern(const std::wstring Pattern)
{
	const std::wregex Troublemakers{ LR"([[\]\^\-+*()?$\\{}])" };
	return std::regex_replace(Pattern, Troublemakers, L" ");
}

std::vector<std::wstring> SplitString(const std::wstring& Glue, const std::wstring& Composite)
{
	std::vector<std::wstring>SplitComponents{};
	const std::wregex ComponentMatch{ GluePattern };
	using rxit = std::wsregex_iterator;
	rxit EmptyRegexIterator{};
	for (rxit PartWalker(Composite.cbegin(), Composite.cend(), ComponentMatch);
		PartWalker != EmptyRegexIterator; ++PartWalker)
	{
		SplitComponents.emplace_back((*PartWalker)[1]);
	}
}