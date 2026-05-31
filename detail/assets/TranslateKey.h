#pragma once

#include <string>

namespace oly::detail
{
	template<typename Key>
	constexpr std::string translate_key(Key key)
	{
		const auto value = static_cast<underlying_or_self_t<Key>>(key);
		constexpr size_t KeySize = 8;
		std::array<char, KeySize> bytes;

		for (size_t i = 0; i < KeySize; ++i)
			bytes[KeySize - 1 - i] = char((value >> (i * KeySize)) & 0xFF);

		return std::string(bytes.begin(), std::find(bytes.begin(), bytes.end(), '\0'));
	}
}
