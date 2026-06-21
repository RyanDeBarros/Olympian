#include "Toolbar.h"

#include "core/editor/ResourceLoader.h"

#include "gui/graphics/Texture.h"
#include "gui/scopes/IDScope.h"

#include <string>

namespace oly::editor
{
	static DrawResult _DrawIconButton(bool& selected, ImVec2 size)
	{
		DrawResult result;
		gui::IDScope scope(&selected);
		if (ImGui::InvisibleButton("##IconButton", size))
		{
			selected = !selected;
			result |= true;
			result.Query();
		}
		return result;
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

	DrawResult Toolbar::DrawIconToggleButton(IconResource selected_icon, IconResource deselected_icon, bool& selected, const char* tooltip)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		DrawResult result = _DrawIconButton(selected, size);
		_HandleIconHovered(pos, size, tooltip);
		DrawIconImage(pos, selected ? selected_icon : deselected_icon, 1.f);
		return result;
	}

	DrawResult Toolbar::DrawIconToggleButton(IconResource icon, bool& selected, const char* tooltip)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		DrawResult result = _DrawIconButton(selected, size);
		_HandleIconHovered(pos, size, tooltip);
		DrawIconImage(pos, icon, selected ? 1.f : 0.3f);
		return result;
	}

	DrawResult Toolbar::DrawIconButton(IconResource icon, const char* tooltip, const char* str_id)
	{
		DrawResult result;
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		if (ImGui::InvisibleButton(str_id, size))
		{
			result |= true;
			result.Query();
		}

		_HandleIconHovered(pos, size, tooltip);
		DrawIconImage(pos, icon, 1.f);
		return result;
	}

	DrawResult Toolbar::DrawHandle(const char* str_id)
	{
		DrawIconImage(ImGui::GetCursorScreenPos(), IconResource::Handle, 1.f);
		DrawResult result = ImGui::InvisibleButton(str_id, ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
		return result.Query();
	}
}
