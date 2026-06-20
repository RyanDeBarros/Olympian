#include "Outline.h"

namespace oly::editor::gui
{
	Outline::Outline(ImU32 color, float border)
		: _start_pos(ImGui::GetCursorScreenPos()), _color(color), _border(border)
	{
	}

	void Outline::Draw() const
	{
		ImVec2 end_pos(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y + ImGui::GetFrameHeight());
		ImGui::GetWindowDrawList()->AddRect(_start_pos, end_pos, _color, 0.f, 0, _border);
	}
}
