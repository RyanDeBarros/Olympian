#include "InlineWidget.h"

#include "core/editor/Editor.h"

#include <imgui.h>

#include <string>

namespace oly::editor::gui
{
	static size_t ROW_COLUMN_COUNTER = 0;
	static size_t MAX_COLUMN_COUNTER = 0;

	enum class LayoutPhase
	{
		None,
		Calc,
		Table
	};

	static LayoutPhase LAYOUT_PHASE = LayoutPhase::None;
	static ImGuiID LAYOUT_ID = 0;

	struct LayoutMapEntry
	{
		size_t last_drawn_frame = 0;
		size_t column_count = 0;
	};

	static std::unordered_map<ImGuiID, LayoutMapEntry> LAYOUTS;

	DrawResult InlineWidget::Draw(const std::span<WidgetComponent> components)
	{
		DrawResult result;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(ImGui::GetStyle().CellPadding.x, 0.f));

		if (LAYOUT_PHASE == LayoutPhase::Table) [[likely]]
		{
			if (ImGui::BeginTable("##InlineWidget", MAX_COLUMN_COUNTER, ImGuiTableFlags_SizingStretchSame,
				ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight())))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				for (auto& component : components)
				{
					// TODO v9.1 setting next item width to -FLT_MIN fills cell but only affects input box, not label -> label goes out of clip rect
					ImGui::SetNextItemWidth(-FLT_MIN);
					result |= component.draw();
				}

				ImGui::EndTable();
			}
		}
		else if (LAYOUT_PHASE == LayoutPhase::Calc)
		{
			// TODO v9.1 instead of EndColumn(), add 'column_span' to WidgetComponent to also allow for initializing weight of column in table column setup
			for (auto& component : components)
				result |= component.draw();

			if (ROW_COLUMN_COUNTER > MAX_COLUMN_COUNTER)
				MAX_COLUMN_COUNTER = ROW_COLUMN_COUNTER;
		}

		ROW_COLUMN_COUNTER = 0;

		ImGui::PopStyleVar();

		return result;
	}

	void InlineWidget::Prune()
	{
		for (auto it = LAYOUTS.begin(); it != LAYOUTS.end(); )
		{
			if (it->second.last_drawn_frame < Editor::Instance().GetFrame())
				it = LAYOUTS.erase(it);
			else
				++it;
		}
	}

	void InlineWidget::StartLayout(ImGuiID id)
	{
		if (LAYOUT_PHASE != LayoutPhase::None)
			EndLayout();

		ROW_COLUMN_COUNTER = 0;

		LAYOUT_ID = id;
		LAYOUT_PHASE = LayoutPhase::Calc;
		MAX_COLUMN_COUNTER = 0;

		auto it = LAYOUTS.find(id);
		if (it != LAYOUTS.end())
		{
			if (it->second.last_drawn_frame + 1 >= Editor::Instance().GetFrame())
			{
				LAYOUT_PHASE = LayoutPhase::Table;
				MAX_COLUMN_COUNTER = it->second.column_count;
			}
		}
	}

	void InlineWidget::EndLayout()
	{
		if (LAYOUT_PHASE == LayoutPhase::Calc)
		{
			if (MAX_COLUMN_COUNTER == 0)
				MAX_COLUMN_COUNTER = 1;
		}

		if (LAYOUT_PHASE != LayoutPhase::None) [[likely]]
			LAYOUTS[LAYOUT_ID] = LayoutMapEntry{ .last_drawn_frame = Editor::Instance().GetFrame(), .column_count = MAX_COLUMN_COUNTER };

		LAYOUT_PHASE = LayoutPhase::None;
	}

	void InlineWidget::EndColumn()
	{
		++ROW_COLUMN_COUNTER;

		if (LAYOUT_PHASE == LayoutPhase::Table) [[likely]]
		{
			if (ROW_COLUMN_COUNTER < MAX_COLUMN_COUNTER)
				ImGui::TableSetColumnIndex(ROW_COLUMN_COUNTER);
		}
	}
}
