#pragma once

namespace oly::detail
{
    using KeyInput = unsigned int;

    constexpr size_t kKeyInputCount = 120;
    extern const KeyInput kKeyInputDefault;
    extern const KeyInput kKeyInputValues[kKeyInputCount];
    extern const char* const kKeyInputNames[kKeyInputCount];
}
