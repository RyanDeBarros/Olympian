#include "Toolbar.h"

#include "core/ResourceLoader.h"

#include "gui/IDScope.h"
#include "gui/Texture.h"

#include <string>

namespace oly::editor
{
	static bool _DrawIconButton(bool& selected, ImVec2 size)
	{
		bool pressed = false;
		gui::IDScope scope(&selected);
		if (ImGui::InvisibleButton("##IconButton", size))
		{
			selected = !selected;
			pressed = true;
		}
		return pressed;
	}

	static void _HandleIconHovered(ImVec2 pos, ImVec2 size, const char* tooltip)
	{
		if (ImGui::IsItemHovered())
		{
			ImGui::GetWindowDrawList()->AddRectFilled(pos, pos + size, ImGui::GetColorU32(ImGuiCol_HeaderHovered, 0.9f), 6.0f);
			ImGui::SetTooltip(tooltip);
		}
	}

	void Toolbar::DrawIconImage(ImVec2 pos, IconResource icon, float tint_alpha)
	{
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		const float shrink = 0.2f;
		const ImVec2 start = pos + 0.5f * size * shrink;
		const ImVec2 end = pos + size * (1.f - 0.5f * shrink);
		ImU32 tint = ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, tint_alpha));
		ImGui::GetWindowDrawList()->AddImage(ResourceLoader::GetTexture(icon).ID(), start, end, ImVec2(0, 0), ImVec2(1, 1), tint);
	}

	bool Toolbar::DrawIconToggleButton(IconResource selected_icon, IconResource deselected_icon, bool& selected, const char* tooltip)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		bool pressed = _DrawIconButton(selected, size);
		_HandleIconHovered(pos, size, tooltip);
		DrawIconImage(pos, selected ? selected_icon : deselected_icon, 1.f);
		return pressed;
	}

	bool Toolbar::DrawIconToggleButton(IconResource icon, bool& selected, const char* tooltip)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		bool pressed = _DrawIconButton(selected, size);
		_HandleIconHovered(pos, size, tooltip);
		DrawIconImage(pos, icon, selected ? 1.f : 0.3f);
		return pressed;
	}

	bool Toolbar::DrawIconButton(IconResource icon, const char* tooltip, const char* str_id)
	{
		bool pressed = false;
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		if (ImGui::InvisibleButton(str_id, size))
			pressed = true;

		_HandleIconHovered(pos, size, tooltip);
		DrawIconImage(pos, icon, 1.f);
		return pressed;
	}
}
