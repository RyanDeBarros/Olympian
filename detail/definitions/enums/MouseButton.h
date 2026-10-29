#pragma once

namespace oly::detail
{
    using MouseButton = unsigned int;

    constexpr size_t kMouseButtonCount = 8;
    extern const MouseButton kMouseButtonDefault;
    extern const MouseButton kMouseButtonValues[kMouseButtonCount];
    extern const char* const kMouseButtonNames[kMouseButtonCount];
}
