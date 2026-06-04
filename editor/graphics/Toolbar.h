#pragma once

namespace oly::editor
{
	enum class Resource : int;

	struct Toolbar
	{
		static void DrawIcon(Resource selected_icon, Resource deselected_icon, bool& selected, const char* tooltip);
		static void DrawIcon(Resource icon, bool& selected, const char* tooltip);
	};
}
