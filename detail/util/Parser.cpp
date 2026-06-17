#include "Parser.h"

#include <charconv>

namespace oly::detail
{
	std::optional<int> stoi(const std::string_view str)
	{
        const char* end = str.data() + str.size();
        if (str.starts_with("0b") || str.starts_with("0B"))
        {
            int value;
            auto result = std::from_chars(str.data() + 2, end, value, 2);

            if (result.ec == std::errc{} && result.ptr == end)
                return value;
        }
        else if (str.starts_with("0x") || str.starts_with("0X"))
        {
            int value;
            auto result = std::from_chars(str.data() + 2, end, value, 16);

            if (result.ec == std::errc{} && result.ptr == end)
                return value;
        }
        else
        {
            int value;
            auto result = std::from_chars(str.data(), end, value, 10);

            if (result.ec == std::errc{} && result.ptr == end)
                return value;
        }

        return std::nullopt;
	}
}
