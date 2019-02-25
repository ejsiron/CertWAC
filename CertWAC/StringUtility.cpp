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
	// strings in this app are few and small; needs modification for JoinedString.resize() pre-calculation and allocation error handling before using in general-purpose code
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

std::vector<std::wstring> SplitString(const std::wstring& GluePattern, const std::wstring& Composite)
{
	std::vector<std::wstring>SplitComponents{};
	if (GluePattern.size())
	{
		std::vector<std::pair<ptrdiff_t, ptrdiff_t>> GlueLocations{};
		const std::wregex ComponentMatch{ GluePattern };
		using rxit = std::wsregex_iterator;
		rxit EmptyRegexIterator{};
		for (rxit PartWalker(Composite.cbegin(), Composite.cend(), ComponentMatch);
			PartWalker != EmptyRegexIterator; ++PartWalker)
		{
			GlueLocations.emplace_back(std::make_pair(PartWalker->position(), (*PartWalker).str().length()));
		}

		size_t CompositeStart{ 0 };
		size_t CompositeSectionLength{ 0 };
		for (auto const& GlueLocation : GlueLocations)
		{
			if (CompositeStart < Composite.length())
			{
				CompositeSectionLength = GlueLocation.first - CompositeStart;
				SplitComponents.emplace_back(Composite.substr(CompositeStart, CompositeSectionLength));
				CompositeStart += CompositeSectionLength + GlueLocation.second;
			}
		}
		size_t LastLength = Composite.length() - CompositeStart;
		SplitComponents.emplace_back(Composite.substr(CompositeStart, LastLength));
	}
	else
		SplitComponents.emplace_back(Composite);
	return SplitComponents;
}