#pragma once

#include <ostream>

namespace oly::detail
{
	enum class PositioningMode
	{
		Relative = 0,
		Absolute
	};

	extern std::ostream& operator<<(std::ostream& os, PositioningMode mode);
}
