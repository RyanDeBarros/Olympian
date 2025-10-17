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
	
	std::string& ltrim(std::string& str)
	{
		if (str.empty())
			return str;

		size_t start = 0;
		while (start < str.size() && std::isspace(str[start]))
			++start;

		str.erase(0, start);

		return str;
	}
	
	std::string& rtrim(std::string& str)
	{
		if (str.empty())
			return str;

		size_t end = str.size();
		while (end > 0 && std::isspace(str[end - 1]))
			--end;

		str.erase(end);

		return str;
	}
	
	std::string& trim(std::string& str)
	{
		if (str.empty())
			return str;

		size_t start = 0;
		while (start < str.size() && std::isspace(str[start]))
			++start;

		size_t end = str.size();
		while (end > start && std::isspace(str[end - 1]))
			--end;

		str.erase(end);
		str.erase(0, start);

		return str;
	}

	std::string& to_lower(std::string& str)
	{
		if (str.empty())
			return str;

		for (char& c : str)
			c = (char)(std::tolower(c));

		return str;
	}

	std::string& to_upper(std::string& str)
	{
		if (str.empty())
			return str;

		for (char& c : str)
			c = (char)(std::toupper(c));

		return str;
	}
}
