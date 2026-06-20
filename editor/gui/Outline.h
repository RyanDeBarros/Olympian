#pragma once

#include <imgui.h>

namespace oly::editor::gui
{
	class Outline
	{
		ImVec2 _start_pos;
		ImU32 _color;
		float _border;

	public:
		Outline(ImU32 color = IM_COL32(255, 0, 0, 255), float border = 1.f);

		void Draw() const;
	};
}
