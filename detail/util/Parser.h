#pragma once

#include <optional>
#include <string_view>

namespace oly::detail
{
	extern std::optional<int> stoi(const std::string_view str);
}
