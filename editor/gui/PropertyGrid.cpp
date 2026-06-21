#include "PropertyGrid.h"

#include <array>

namespace oly::editor::gui
{
	static std::vector<WidgetComponent> KEY_COMPONENTS;
	static DrawResult KEY_DRAW_RESULT;

	static std::vector<WidgetComponent> VALUE_COMPONENTS;
	static DrawResult VALUE_DRAW_RESULT;
	
	static std::vector<WidgetComponent> RESET_COMPONENTS;
	static DrawResult RESET_DRAW_RESULT;

	static bool DIRTY_GRID = false;

	DrawResult PropertyGrid::Key::GetDrawResult()
	{
		return KEY_DRAW_RESULT;
	}

	void PropertyGrid::Key::AddComponent(WidgetComponent component)
	{
		KEY_COMPONENTS.push_back(std::move(component));
	}

	void PropertyGrid::Key::SameLine()
	{
		AddComponent({ []() -> DrawResult { ImGui::SameLine(); return false; } });
	}

	DrawResult PropertyGrid::Value::GetDrawResult()
	{
		return VALUE_DRAW_RESULT;
	}

	void PropertyGrid::Value::AddComponent(WidgetComponent component)
	{
		VALUE_COMPONENTS.push_back(std::move(component));
	}

	void PropertyGrid::Value::SameLine()
	{
		AddComponent({ []() -> DrawResult { ImGui::SameLine(); return false; } });
	}

	DrawResult PropertyGrid::Reset::GetDrawResult()
	{
		return RESET_DRAW_RESULT;
	}

	void PropertyGrid::Reset::AddComponent(WidgetComponent component)
	{
		RESET_COMPONENTS.push_back(std::move(component));
	}

	void PropertyGrid::Reset::SameLine()
	{
		AddComponent({ []() -> DrawResult { ImGui::SameLine(); return false; } });
	}

	static void SubmitCell(int i, std::vector<WidgetComponent>& components, DrawResult& draw_result)
	{
		ImGui::TableSetColumnIndex(i);

		DrawResult result;
		for (const WidgetComponent& component : components)
			result |= component.draw();

		components.clear();
		draw_result = result;
	}

	// TODO v9.1 if table is expanded dynamically with Subform, key column width doesn't adapt - check animated subform for texture document

	// TODO v9.1 draw value cell first, so height can be determined and the key cell aligned to middle vertically?

	void PropertyGrid::SubmitRow()
	{
		ImGui::TableNextRow();

		SubmitCell(0, KEY_COMPONENTS, KEY_DRAW_RESULT);
		SubmitCell(1, VALUE_COMPONENTS, VALUE_DRAW_RESULT);
		SubmitCell(2, RESET_COMPONENTS, RESET_DRAW_RESULT);

		DIRTY_GRID |= DirtyRow();
	}

	bool PropertyGrid::DirtyRow()
	{
		return Value::GetDrawResult().IsDirty() || Reset::GetDrawResult().IsDirty();
	}

	void PropertyGrid::Clear()
	{
		KEY_COMPONENTS.clear();
		KEY_DRAW_RESULT = {};

		VALUE_COMPONENTS.clear();
		VALUE_DRAW_RESULT = {};

		RESET_COMPONENTS.clear();
		RESET_DRAW_RESULT = {};

		DIRTY_GRID = false;
	}

	bool PropertyGrid::DirtyGrid()
	{
		return DIRTY_GRID;
	}

	bool PropertyGrid::BeginTable()
	{
		if (ImGui::BeginTable("", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
			return true;
		}
		else
			return false;
	}
}
