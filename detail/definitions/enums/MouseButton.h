#pragma once

namespace oly::detail
{
	typedef unsigned int MouseButton;

	constexpr size_t MOUSE_BUTTON_COUNT = 8;
	extern MouseButton MOUSE_BUTTON_DEFAULT;
	extern MouseButton MOUSE_BUTTON_VALUES[MOUSE_BUTTON_COUNT];
	extern const char* MOUSE_BUTTON_NAMES[MOUSE_BUTTON_COUNT];
}
