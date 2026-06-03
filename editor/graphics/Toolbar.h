#pragma once

namespace oly::editor
{
	enum class Resource : int;

	struct Toolbar
	{
		static void DrawIcon(Resource selected_icon, Resource deselected_icon, bool& selected);
	};
}
