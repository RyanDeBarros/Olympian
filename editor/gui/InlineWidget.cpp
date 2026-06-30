#include "InlineWidget.h"

#include <imgui.h>

namespace oly::editor::gui
{
	DrawResult InlineWidget::Draw(const std::span<WidgetComponent> components)
	{
		DrawResult result;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(ImGui::GetStyle().CellPadding.x, 0.f));

		if (ImGui::BeginTable("##InlineWidget", components.size(), ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_PreciseWidths,
			ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight())))
		{
			ImGui::TableNextRow();

			for (auto& component : components)
			{
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(-FLT_MIN);
				result |= component.draw();
				ImGui::PopItemWidth();
			}

			ImGui::EndTable();
		}

		ImGui::PopStyleVar();

		return result;
	}
}
