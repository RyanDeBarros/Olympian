#pragma once

namespace oly::detail
{
    using InputMod = unsigned int;

    constexpr size_t kInputModCount = 6;
    extern const InputMod kInputModDefault;
    extern const InputMod kInputModValues[kInputModCount];
    extern const char* const kInputModNames[kInputModCount];
}
