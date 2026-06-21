#pragma once

#include "gui/DrawResult.h"

namespace oly::editor
{
	enum class IconResource : int;

	struct Toolbar
	{
		static void DrawIconImage(ImVec2 pos, IconResource icon, float tint_alpha = 1.f);
		static DrawResult DrawIconToggleButton(IconResource selected_icon, IconResource deselected_icon, bool& selected, const char* tooltip);
		static DrawResult DrawIconToggleButton(IconResource icon, bool& selected, const char* tooltip);
		static DrawResult DrawIconButton(IconResource icon, const char* tooltip, const char* str_id = "");
		static DrawResult DrawHandle(const char* str_id = "");
	};
}
