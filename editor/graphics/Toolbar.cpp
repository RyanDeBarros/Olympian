#include "Toolbar.h"

#include "core/ResourceLoader.h"

#include <imgui.h>

namespace oly::editor
{
	void Toolbar::DrawIcon(Resource selected_icon, Resource deselected_icon, bool& selected)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(32, 32);
		const float shrink = 0.2f;
		const ImVec2 start = pos + 0.5f * size * shrink;
		const ImVec2 end = pos + size * (1.f - 0.5f * shrink);

		ImGui::PushID(&selected);
		if (ImGui::InvisibleButton("##IconButton", size))
			selected = !selected;
		ImGui::PopID();

		ImDrawList* dl = ImGui::GetWindowDrawList();

		if (ImGui::IsItemHovered())
			dl->AddRectFilled(pos, pos + size, ImGui::GetColorU32(ImGuiCol_HeaderHovered, 0.9f), 6.0f);

		dl->AddImage(ResourceLoader::GetTexture(selected ? selected_icon : deselected_icon).ID(), start, end);
	}
}
