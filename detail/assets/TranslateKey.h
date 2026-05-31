#pragma once

#include <string>

namespace oly::detail
{
	constexpr size_t KeySize = 8;
	enum class Key : unsigned long long;

	extern std::string translate_key(Key key);
}
