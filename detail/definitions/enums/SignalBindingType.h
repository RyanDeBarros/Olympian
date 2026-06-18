#pragma once

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
}
