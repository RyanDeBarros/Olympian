#pragma once

#include <imgui.h>

namespace oly::editor::gui
{
	extern void FloatControl(const char* label, float& value, const float item_width, const float min, const float max, const char* format = "%.3f", bool logarithmic = false);
}
