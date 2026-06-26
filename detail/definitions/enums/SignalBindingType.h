#pragma once

#include <ostream>

namespace oly::detail
{
	enum class SignalBindingType
	{
		Key = 0,
		MouseButton,
		GamepadButton,
		GamepadAxis1D,
		GamepadAxis2D,
		CursorPos,
		Scroll
	};

	extern std::ostream& operator<<(std::ostream& os, SignalBindingType type);
}
