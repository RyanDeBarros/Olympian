#pragma once

#include <imgui.h>

namespace oly::editor
{
	enum class Resource : int;

	struct Toolbar
	{
		static void DrawIconImage(ImVec2 pos, Resource icon, float tint_alpha = 1.f);
		static void DrawIconToggleButton(Resource selected_icon, Resource deselected_icon, bool& selected, const char* tooltip);
		static void DrawIconToggleButton(Resource icon, bool& selected, const char* tooltip);
		static bool DrawIconButton(Resource icon, const char* tooltip, int id_counter);
		static bool DrawIconButton(Resource icon, const char* tooltip, const void* id);
	};
}
