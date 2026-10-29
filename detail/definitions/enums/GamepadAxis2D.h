#pragma once

#include <ostream>

namespace oly::detail
{
    enum class GamepadAxis2D
    {
        LeftXY = 0,
        RightXY,
        _first = LeftXY,
        _last = RightXY
    };

    extern std::ostream& operator<<(std::ostream& os, GamepadAxis2D axis);
}
