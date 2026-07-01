#include "Outline.h"

namespace oly::editor::gui
{
	Outline::Outline()
		: _start_pos(ImGui::GetCursorScreenPos())
	{
	}

	void Outline::Draw(ImU32 color, float border) const
	{
		ImVec2 end_pos(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y + ImGui::GetFrameHeight());
		ImGui::GetWindowDrawList()->AddRect(_start_pos, end_pos, color, 0.f, 0, border);
	}
}
