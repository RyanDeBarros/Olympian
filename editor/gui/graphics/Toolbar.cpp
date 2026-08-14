#include "Toolbar.h"

#include "core/editor/ResourceLoader.h"

#include <string>

namespace oly::editor
{
	static imtk::item_result _DrawIconButton(bool& selected, ImVec2 size)
	{
		imtk::item_result result;
		imtk::id_scope scope(&selected);
		if (ImGui::InvisibleButton("##IconButton", size))
		{
			selected = !selected;
			result = imtk::item_result::query(true);
		}
		else
			result = imtk::item_result::query(false);
		return result;
	}

	static void _HandleIconHovered(imtk::item_state state, ImVec2 pos, ImVec2 size, const char* tooltip)
	{
		if (state.hovered())
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
		ImGui::GetWindowDrawList()->AddImage(ResourceLoader::GetTexture(icon).id(), start, end, ImVec2(0, 0), ImVec2(1, 1), tint);
	}

	imtk::item_result Toolbar::DrawIconToggleButton(IconResource selected_icon, IconResource deselected_icon, bool& selected, const char* tooltip, IconSettings settings)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		imtk::item_result result = _DrawIconButton(selected, size);
		_HandleIconHovered(result.state, pos, size, tooltip);
		DrawIconImage(pos, selected ? selected_icon : deselected_icon, settings);
		return result;
	}

	imtk::item_result Toolbar::DrawIconToggleButton(IconResource icon, bool& selected, const char* tooltip, IconSettings settings)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		imtk::item_result result = _DrawIconButton(selected, size);
		_HandleIconHovered(result.state, pos, size, tooltip);
		if (!selected)
			settings.tint_alpha *= 0.3f;
		DrawIconImage(pos, icon, settings);
		return result;
	}

	imtk::item_result Toolbar::DrawIconButton(IconResource icon, const char* tooltip, const char* str_id, IconSettings settings)
	{
		const ImVec2 pos = ImGui::GetCursorScreenPos();
		const ImVec2 size = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		auto result = imtk::item_result::query(ImGui::InvisibleButton(str_id, size));

		_HandleIconHovered(result.state, pos, size, tooltip);
		DrawIconImage(pos, icon, settings);
		return result;
	}

	imtk::item_result Toolbar::DrawHandle(const char* str_id)
	{
		DrawIconImage(ImGui::GetCursorScreenPos(), IconResource::Handle, {});
		return imtk::item_result::query(ImGui::InvisibleButton(str_id, ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())));
	}

	imtk::item_result Toolbar::IconMenuItem(std::string label, IconResource icon, IconSettings settings)
	{
		auto result = imtk::item_result(ImGui::MenuItem(("   " + label).c_str()));
		Toolbar::DrawIconImage(ImGui::GetItemRectMin() + ImVec2(2.f, 0.f), icon, settings);
		return result;
	}
}
