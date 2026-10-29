#include "MouseButton.h"

#include <GLFW/glfw3.h>

namespace oly::detail
{
    const MouseButton kMouseButtonDefault = GLFW_MOUSE_BUTTON_LEFT;

    const MouseButton kMouseButtonValues[kMouseButtonCount] = {
        GLFW_MOUSE_BUTTON_1,
        GLFW_MOUSE_BUTTON_2,
        GLFW_MOUSE_BUTTON_3,
        GLFW_MOUSE_BUTTON_4,
        GLFW_MOUSE_BUTTON_5,
        GLFW_MOUSE_BUTTON_6,
        GLFW_MOUSE_BUTTON_7,
        GLFW_MOUSE_BUTTON_8
    };

    const char* const kMouseButtonNames[kMouseButtonCount] = {
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
