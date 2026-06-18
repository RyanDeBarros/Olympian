#pragma once

#include <imgui.h>

namespace oly::editor
{
	enum class IconResource : int;

	struct Toolbar
	{
		static void DrawIconImage(ImVec2 pos, IconResource icon, float tint_alpha = 1.f);
		static bool DrawIconToggleButton(IconResource selected_icon, IconResource deselected_icon, bool& selected, const char* tooltip);
		static bool DrawIconToggleButton(IconResource icon, bool& selected, const char* tooltip);
		static bool DrawIconButton(IconResource icon, const char* tooltip, const char* str_id = "");
		static bool DrawHandle(const char* str_id = "");
	};
}
