#include "InputMod.h"

#include <GLFW/glfw3.h>

namespace oly::detail
{
    const InputMod kInputModDefault = 0;

    const InputMod kInputModValues[kInputModCount] = {
        GLFW_MOD_SHIFT,
        GLFW_MOD_CONTROL,
        GLFW_MOD_ALT,
        GLFW_MOD_SUPER,
        GLFW_MOD_CAPS_LOCK,
        GLFW_MOD_NUM_LOCK
    };

    const char* const kInputModNames[kInputModCount] = {
        "Shift",
        "Control",
        "Alt",
        "Super",
        "Caps lock",
        "Num lock"
    };
}
