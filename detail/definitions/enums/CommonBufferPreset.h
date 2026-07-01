#pragma once

#include <ostream>

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

	extern std::ostream& operator<<(std::ostream& os, CommonBufferPreset preset);
}
