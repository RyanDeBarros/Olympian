#pragma once

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
		static imtk::item_result DrawIconToggleButton(IconResource selected_icon, IconResource deselected_icon, bool& selected, const char* tooltip, IconSettings settings = {});
		static imtk::item_result DrawIconToggleButton(IconResource icon, bool& selected, const char* tooltip, IconSettings settings = {});
		static imtk::item_result DrawIconButton(IconResource icon, const char* tooltip, const char* str_id = "", IconSettings settings = {});
		static imtk::item_result DrawHandle(const char* str_id = "");
		static imtk::item_result IconMenuItem(std::string label, IconResource icon, IconSettings settings = {});
	};
}
