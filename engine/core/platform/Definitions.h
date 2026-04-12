#pragma once

namespace oly::input
{
	enum class SignalBindingType
	{
		Key = 0x0,
		MouseButton = 0x1,
		GamepadButton = 0x2,
		GamepadAxis1D = 0x3,
		GamepadAxis2D = 0x4,
		CursorPos = 0x5,
		Scroll = 0x6
	};
}
