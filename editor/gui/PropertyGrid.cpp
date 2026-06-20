#include "PropertyGrid.h"

#include <array>

namespace oly::editor::gui
{
	static PropertyGrid::Column COLUMN = PropertyGrid::Column::Key;
	static std::array<std::vector<WidgetComponent>, PropertyGrid::Column::_C> COMPONENTS;
	static std::array<DrawResult, PropertyGrid::Column::_C> DRAW_RESULTS;
	static bool DIRTY_GRID = false;

	void PropertyGrid::SetColumn(Column column)
	{
		COLUMN = column;
	}

	// TODO v9.1 draw value cell first, so height can be determined and the key cell aligned to middle vertically?
	
	void PropertyGrid::SubmitRow()
	{
		ImGui::TableNextRow();

		for (int i = 0; i < Column::_C; ++i)
		{
			ImGui::TableSetColumnIndex(i);

			DrawResult result;
			for (const WidgetComponent& component : COMPONENTS[i])
				result |= component.draw();

			COMPONENTS[i].clear();
			DRAW_RESULTS[i] = result;
		}

		DIRTY_GRID |= DirtyValue();
	}

	DrawResult PropertyGrid::GetDrawResult(Column column)
	{
		return DRAW_RESULTS[column];
	}

	void PropertyGrid::AddComponent(WidgetComponent component)
	{
		COMPONENTS[COLUMN].push_back(std::move(component));
	}

	void PropertyGrid::SameLine()
	{
		AddComponent({ []() -> DrawResult { ImGui::SameLine(); return false; } });
	}

	bool PropertyGrid::DirtyValue()
	{
		return GetDrawResult(Column::Value) || GetDrawResult(Column::Reset);
	}

	void PropertyGrid::Clear()
	{
		for (int i = 0; i < Column::_C; ++i)
		{
			COMPONENTS[i].clear();
			DRAW_RESULTS[i] = {};
		}
		DIRTY_GRID = false;
	}

	bool PropertyGrid::DirtyGrid()
	{
		return DIRTY_GRID;
	}
}
