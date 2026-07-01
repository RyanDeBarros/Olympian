#pragma once

#include <ostream>

namespace oly::detail
{
	enum class SpritesheetParamType : char
	{
		Index = 0,
		Pixel
	};

	extern std::ostream& operator<<(std::ostream& os, SpritesheetParamType type);
}
