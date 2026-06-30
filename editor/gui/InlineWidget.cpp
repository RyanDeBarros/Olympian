#include "InlineWidget.h"

#include <imgui.h>

#include <string>

namespace oly::editor::gui
{
	static float GetSubItemWidth(const float remaining_width, const size_t remaining_count)
	{
		return remaining_count > 0 ? (remaining_width - ImGui::GetStyle().ItemSpacing.x * (remaining_count - 1)) / remaining_count : remaining_width;
	}

	// TODO v9.1 ideal layout within property grid value column should be:
	// 
	// 1. InputInt
	// 2. Checkbox+InputInt
	// 3. 2 InputInts
	// 
	// [.....]
	// [.][..]
	// [.....][.....]
	// 
	// Basically, in each Subform, divide the value column into N subcolumn segments -> use ImGuiTableFlags_SizingStretchSame in InlineWidget. Instead of some components stretching, define each component to take up a subcolumn. That is, a checkbox+inputint would need to be 1 single component.
	// Can implement in two ways:
	// 1. generator in Subform that determines the N = max(size(components) for each row in subform) -> expensive
	// 2. Map ImGuiID to persistent state -> on first runthough, use old 'GetSubItemWidth()' logic (or assume N = 1 ?). While drawing row by row, compute N. On next frame, use the new N and update it for the next frame if needed.

	DrawResult InlineWidget(const std::span<WidgetComponent> components)
	{
		DrawResult result;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(ImGui::GetStyle().CellPadding.x, 0.f));

		if (ImGui::BeginTable("##InlineWidget", components.size(), ImGuiTableFlags_SizingStretchSame, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight())))
		{
			for (size_t i = 0; i < components.size(); ++i)
				ImGui::TableSetupColumn(("##" + std::to_string(i)).c_str(), components[i].stretch ? ImGuiTableColumnFlags_WidthStretch : ImGuiTableColumnFlags_WidthFixed, components[i].stretch ? 1.f : 0.f);

			ImGui::TableNextRow();

			for (auto& component : components)
			{
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(-FLT_MIN);
				result |= component.draw();
			}

			ImGui::EndTable();
		}

		ImGui::PopStyleVar();

		//float avail_width = ImGui::GetContentRegionAvail().x;

		//size_t remaining_count = components.size();
		//for (size_t i = 0; i < components.size(); ++i)
		//{
		//	const float item_width = GetSubItemWidth(avail_width, remaining_count--);
		//	ImGui::SetNextItemWidth(item_width);

		//	const float start_x = ImGui::GetCursorPosX();
		//	result |= components[i].draw();
		//	if (i + 1 < components.size())
		//		ImGui::SameLine();

		//	avail_width -= ImGui::GetCursorPosX() - start_x;
		//}

		return result;
	}
}
