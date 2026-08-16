#pragma once

#include <imgui.h>

// TODO v9.3 move to imtk
namespace oly::editor::gui
{
	struct Overlay
	{
		static void QuadError(ImVec2 rect_start, ImVec2 rect_end);
		static void QuadWarning(ImVec2 rect_start, ImVec2 rect_end);
	};
}
