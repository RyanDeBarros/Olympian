#include "STLUtils.h"

namespace oly
{
	std::vector<std::string_view> split(std::string_view sv, char delimiter)
	{
		std::vector<std::string_view> result;
		size_t start = 0;
		while (start < sv.size())
		{
			size_t end = sv.find(delimiter, start);
			if (end == std::string_view::npos)
			{
				result.emplace_back(sv.substr(start));
				break;
			}
			result.emplace_back(sv.substr(start, end - start));
			start = end + 1;
		}
		return result;
	}
}
