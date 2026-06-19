#include "Parser.h"

#include <charconv>

namespace oly
{
    std::optional<int> stoi_direct(const std::string_view str, const int base)
    {
        int value;
        auto result = std::from_chars(str.data(), str.data() + str.size(), value, base);

        if (result.ec == std::errc{} && result.ptr == str.data() + str.size())
            return value;
        else
            return std::nullopt;
    }

	std::optional<int> stoi(const std::string_view str)
	{
        if (str.starts_with("0b") || str.starts_with("0B"))
            return stoi_direct(str.substr(2), 2);
        else if (str.starts_with("0x") || str.starts_with("0X"))
            return stoi_direct(str.substr(2), 16);
        else
            return stoi_direct(str, 10);
	}

    std::optional<int> stocdpt(const std::string_view str)
    {
        if (str.size() >= 3)
        {
            std::string_view prefix = str.substr(0, 2);
            if (prefix == "U+" || prefix == "0x" || prefix == "0X" || prefix == "\\u" || prefix == "\\U" || prefix == "0h")
                return stoi_direct(str.substr(2), 16);
            else if ((str.substr(0, 3) == "&#x" || str.substr(0, 3) == "&#X") && str.ends_with(';'))
                return stoi_direct(str.substr(3, str.size() - 3 - 1), 16);
            else if (str.substr(0, 2) == "&#" && str.ends_with(';'))
                return stoi_direct(str.substr(2, str.size() - 2 - 1), 10);
            else
                return std::nullopt;
        }
        else if (str.empty() || str.size() == 2)
            return std::nullopt;
        else // size == 1
            return static_cast<int>(str.front());
    }
}
