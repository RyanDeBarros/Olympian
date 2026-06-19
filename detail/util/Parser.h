#pragma once

#include <optional>
#include <string_view>

namespace oly
{
	extern std::optional<int> stoi_direct(const std::string_view str, const int base);
	extern std::optional<int> stoi(const std::string_view str);
	extern std::optional<int> stocdpt(const std::string_view str);
}
