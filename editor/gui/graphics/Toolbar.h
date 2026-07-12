#pragma once

#include "gui/DrawResult.h"

#include <imtk.hpp>

#include <string>

namespace oly::editor
{
	enum class IconResource : int;

	struct Toolbar
	{
		struct IconSettings
		{
			float shrink = 0.f;
			float tint_alpha = 1.f;
		};

		static void DrawIconImage(ImVec2 pos, IconResource icon, IconSettings settings = {});
		static DrawResult DrawIconToggleButton(IconResource selected_icon, IconResource deselected_icon, bool& selected, const char* tooltip, IconSettings settings = {});
		static DrawResult DrawIconToggleButton(IconResource icon, bool& selected, const char* tooltip, IconSettings settings = {});
		static DrawResult DrawIconButton(IconResource icon, const char* tooltip, const char* str_id = "", IconSettings settings = {});
		static DrawResult DrawHandle(const char* str_id = "");
		static DrawResult IconMenuItem(std::string label, IconResource icon, IconSettings settings = {});
	};
}
