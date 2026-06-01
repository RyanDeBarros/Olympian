#include "TranslateKey.h"

#include "definitions/Keys.h"

#include <array>

namespace oly::detail
{
	// TODO v7 switch signatures of decode/encode

	std::string encode_key(Key key)
	{
		const auto value = static_cast<unsigned long long>(key);
		std::array<char, KeySize> bytes{};

		for (size_t i = 0; i < KeySize; ++i)
			bytes[KeySize - 1 - i] = char((value >> (i * 8)) & 0xFF);

		return std::string(bytes.begin(), std::find(bytes.begin(), bytes.end(), '\0'));
	}

	Key decode_key(const std::string& code)
	{
		unsigned long long value = 0;

		for (size_t i = 0; i < KeySize; ++i)
		{
			unsigned char byte = 0;
			if (i < code.size())
				byte = static_cast<unsigned char>(code[i]);

			value |= static_cast<unsigned long long>(byte) << ((KeySize - 1 - i) * 8);
		}

		return static_cast<Key>(value);
	}
}
