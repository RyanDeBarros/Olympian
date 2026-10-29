#pragma once

namespace oly::detail
{
	using KeyInput = unsigned int;

	constexpr size_t KEY_INPUT_COUNT = 120;
	extern KeyInput KEY_INPUT_DEFAULT;
	extern KeyInput KEY_INPUT_VALUES[KEY_INPUT_COUNT];
	extern const char* KEY_INPUT_NAMES[KEY_INPUT_COUNT];
}
