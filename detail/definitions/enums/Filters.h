#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace oly::detail
{
	constexpr size_t MIN_FILTER_COUNT = 6;
	extern const GLenum MIN_FILTER_VALUES[MIN_FILTER_COUNT];
	extern const char* MIN_FILTER_NAMES[MIN_FILTER_COUNT];

	constexpr size_t MAG_FILTER_COUNT = 2;
	extern const GLenum MAG_FILTER_VALUES[MAG_FILTER_COUNT];
	extern const char* MAG_FILTER_NAMES[MAG_FILTER_COUNT];

	constexpr size_t WRAP_COUNT = 5;
	extern const GLenum WRAP_VALUES[WRAP_COUNT];
	extern const char* WRAP_NAMES[WRAP_COUNT];
}
