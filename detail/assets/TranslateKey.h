#pragma once

#include <string>

namespace oly::detail
{
	constexpr size_t KeySize = 8;
	enum class Key : unsigned long long;

	extern std::string decode_key(Key key);
	extern Key encode_key(const std::string& code);
}
