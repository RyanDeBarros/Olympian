#include "Toolbar.h"

#include "core/ResourceLoader.h"

#include <imgui.h>

namespace oly::editor
{
	static void DrawIconButton(bool& selected, ImVec2 size)
	{
		ImGui::PushID(&selected);
		if (ImGui::InvisibleButton("##IconButton", size))
			selected = !selected;
		ImGui::PopID();
	}

	static void HandleIconHovered(ImVec2 pos, ImVec2 size, const char* tooltip)
	{
		if (ImGui::IsItemHovered())
		{
			ImGui::GetWindowDrawList()->AddRectFilled(pos, pos + size, ImGui::GetColorU32(ImGuiCol_HeaderHovered, 0.9f), 6.0f);
			ImGui::SetTooltip(tooltip);
		}
	}

	static void DrawIconImage(ImVec2 pos, ImVec2 size, Resource icon, float tint_alpha)
	{
		const float shrink = 0.2f;
		const ImVec2 start = pos + 0.5f * size * shrink;
		const ImVec2 end = pos + size * (1.f - 0.5f * shrink);
		ImU32 tint = ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, tint_alpha));
		ImGui::GetWindowDrawList()->AddImage(ResourceLoader::GetTexture(icon).ID(), start, end);
	}

	void Toolbar::DrawIcon(Resource selected_icon, Resource deselected_icon, bool& selected, const char* tooltip)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(32, 32);
		DrawIconButton(selected, size);
		HandleIconHovered(pos, size, tooltip);
		DrawIconImage(pos, size, selected ? selected_icon : deselected_icon, 1.f);
	}

	void Toolbar::DrawIcon(Resource icon, bool& selected, const char* tooltip)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(32, 32);
		DrawIconButton(selected, size);
		HandleIconHovered(pos, size, tooltip);
		DrawIconImage(pos, size, icon, selected ? 1.f : 0.3f);
	}
}
