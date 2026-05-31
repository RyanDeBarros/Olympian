#include "TranslateKey.h"

#include "definitions/Keys.h"

#include <array>

namespace oly::detail
{
	std::string translate_key(Key key)
	{
		const auto value = static_cast<unsigned long long>(key);
		std::array<char, KeySize> bytes{};

		for (size_t i = 0; i < KeySize; ++i)
			bytes[KeySize - 1 - i] = char((value >> (i * 8)) & 0xFF);

		return std::string(bytes.begin(), std::find(bytes.begin(), bytes.end(), '\0'));
	}
}
