#include "InlineWidget.h"

#include <imtk.hpp>

namespace oly::editor::gui
{
	DrawResult InlineWidget::Draw(const std::span<WidgetComponent> components)
	{
		DrawResult result;

		imtk::style_var cell_padding(ImGuiStyleVar_CellPadding, ImVec2(ImGui::GetStyle().CellPadding.x, 0.f));

		if (ImGui::BeginTable("##InlineWidget", components.size(), ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_PreciseWidths,
			ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight())))
		{
			ImGui::TableNextRow();

			for (auto& component : components)
			{
				ImGui::TableNextColumn();
				if (auto _ = imtk::item_width_scope(imtk::expand_item_width{}))
					result |= component.draw();
			}

			ImGui::EndTable();
		}

		return result;
	}
}
