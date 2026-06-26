#pragma once

#include <ostream>

namespace oly::detail
{
	enum class GamepadAxis2D
	{
		LeftXY = 0,
		RightXY,
		_FIRST = LeftXY,
		_LAST = RightXY
	};

	extern std::ostream& operator<<(std::ostream& os, GamepadAxis2D axis);
}
