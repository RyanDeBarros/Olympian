#include "InputMod.h"

#include <GLFW/glfw3.h>

namespace oly::detail
{
	InputMod INPUT_MOD_DEFAULT = 0;

	InputMod INPUT_MOD_VALUES[INPUT_MOD_COUNT] = {
		GLFW_MOD_SHIFT,
		GLFW_MOD_CONTROL,
		GLFW_MOD_ALT,
		GLFW_MOD_SUPER,
		GLFW_MOD_CAPS_LOCK,
		GLFW_MOD_NUM_LOCK
	};

	extern const char* INPUT_MOD_NAMES[INPUT_MOD_COUNT] = {
		"Shift",
		"Control",
		"Alt",
		"Super",
		"Caps lock",
		"Num lock"
	};
}
