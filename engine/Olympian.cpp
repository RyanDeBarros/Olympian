#include "Olympian.h"

namespace oly
{
    platform::Window& context_window()
    {
        return context::get_platform().window();
    }

    platform::Gamepad& context_gamepad()
    {
        return context::get_platform().gamepad();
    }

    rendering::Camera2D& default_camera()
    {
        return *rendering::Camera2DRef(kRefDefault);
    }
}
