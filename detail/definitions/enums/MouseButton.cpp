#include "MouseButton.h"

#include <GLFW/glfw3.h>

namespace oly::detail
{
	MouseButton MOUSE_BUTTON_DEFAULT = GLFW_MOUSE_BUTTON_LEFT;

	MouseButton MOUSE_BUTTON_VALUES[MOUSE_BUTTON_COUNT] = {
		GLFW_MOUSE_BUTTON_1,
		GLFW_MOUSE_BUTTON_2,
		GLFW_MOUSE_BUTTON_3,
		GLFW_MOUSE_BUTTON_4,
		GLFW_MOUSE_BUTTON_5,
		GLFW_MOUSE_BUTTON_6,
		GLFW_MOUSE_BUTTON_7,
		GLFW_MOUSE_BUTTON_8
	};
	
	const char* MOUSE_BUTTON_NAMES[MOUSE_BUTTON_COUNT] = {
		"Left mouse button",
		"Right mouse button",
		"Middle mouse button",
		"Mouse button #4",
		"Mouse button #5",
		"Mouse button #6",
		"Mouse button #7",
		"Mouse button #8"
	};
}
