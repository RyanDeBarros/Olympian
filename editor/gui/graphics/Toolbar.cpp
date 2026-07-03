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
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::GetWindowDrawList()->AddRectFilled(pos, pos + size, ImGui::GetColorU32(ImGuiCol_HeaderHovered, 0.9f), 6.0f);
			if (tooltip)
				ImGui::SetTooltip(tooltip);
		}
	}

	void Toolbar::DrawIconImage(ImVec2 pos, IconResource icon, IconSettings settings)
	{
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		const ImVec2 start = pos + 0.5f * size * settings.shrink;
		const ImVec2 end = pos + size * (1.f - 0.5f * settings.shrink);
		ImU32 tint = ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, settings.tint_alpha));
		ImGui::GetWindowDrawList()->AddImage(ResourceLoader::GetTexture(icon).ID(), start, end, ImVec2(0, 0), ImVec2(1, 1), tint);
	}

	DrawResult Toolbar::DrawIconToggleButton(IconResource selected_icon, IconResource deselected_icon, bool& selected, const char* tooltip, IconSettings settings)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		DrawResult result = _DrawIconButton(selected, size);
		_HandleIconHovered(pos, size, tooltip);
		DrawIconImage(pos, selected ? selected_icon : deselected_icon, settings);
		return result;
	}

	DrawResult Toolbar::DrawIconToggleButton(IconResource icon, bool& selected, const char* tooltip, IconSettings settings)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		DrawResult result = _DrawIconButton(selected, size);
		_HandleIconHovered(pos, size, tooltip);
		if (!selected)
			settings.tint_alpha *= 0.3f;
		DrawIconImage(pos, icon, settings);
		return result;
	}

	DrawResult Toolbar::DrawIconButton(IconResource icon, const char* tooltip, const char* str_id, IconSettings settings)
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
		DrawIconImage(pos, icon, settings);
		return result;
	}

	DrawResult Toolbar::DrawHandle(const char* str_id)
	{
		DrawIconImage(ImGui::GetCursorScreenPos(), IconResource::Handle, {});
		DrawResult result = ImGui::InvisibleButton(str_id, ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
		return result.Query();
	}

	DrawResult Toolbar::IconMenuItem(std::string label, IconResource icon, IconSettings settings)
	{
		DrawResult result = ImGui::MenuItem(("   " + label).c_str());
		result.Query();
		ImGui::SameLine();
		Toolbar::DrawIconImage(ImGui::GetItemRectMin() + ImVec2(2.f, 0.f), icon, settings);
		return result;
	}
}
