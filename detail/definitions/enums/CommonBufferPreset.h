#pragma once

namespace oly::detail
{
	enum class CommonBufferPreset
	{
		Common,
		AlphaNumeric,
		Numeric,
		Alphabet,
		AlphabetLowercase,
		AlphabetUppercase
	};

	extern const char* buffer_of(CommonBufferPreset preset);
}
