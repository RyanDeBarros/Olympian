#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace oly::detail
{
    constexpr size_t kMinFilterCount = 6;
    extern const GLenum kMinFilterValues[kMinFilterCount];
    extern const char* const kMinFilterNames[kMinFilterCount];

    constexpr size_t kMagFilterCount = 2;
    extern const GLenum kMagFilterValues[kMagFilterCount];
    extern const char* const kMagFilterNames[kMagFilterCount];

    constexpr size_t kWrapCount = 5;
    extern const GLenum kWrapValues[kWrapCount];
    extern const char* const kWrapNames[kWrapCount];
}
