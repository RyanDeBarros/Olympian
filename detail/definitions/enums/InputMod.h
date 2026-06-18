#pragma once

namespace oly::detail
{
	typedef unsigned int InputMod;

	constexpr size_t INPUT_MOD_COUNT = 6;
	extern InputMod INPUT_MOD_DEFAULT;
	extern InputMod INPUT_MOD_VALUES[INPUT_MOD_COUNT];
	extern const char* INPUT_MOD_NAMES[INPUT_MOD_COUNT];
}
