#include "InlineWidget.h"

#include <imgui.h>

namespace oly::editor::gui
{
	static float GetSubItemWidth(const float remaining_width, const size_t remaining_count)
	{
		return remaining_count > 0 ? (remaining_width - ImGui::GetStyle().ItemSpacing.x * (remaining_count - 1)) / remaining_count : remaining_width;
	}

	// TODO v9.1 better implementation -> use inner table since some components like vertical separator ruin the width distribution
	DrawResult InlineWidget(const std::span<WidgetComponent> components)
	{
		DrawResult result;

		float avail_width = ImGui::GetContentRegionAvail().x;

		size_t remaining_count = components.size();
		for (size_t i = 0; i < components.size(); ++i)
		{
			const float item_width = GetSubItemWidth(avail_width, remaining_count--);
			ImGui::SetNextItemWidth(item_width);

			const float start_x = ImGui::GetCursorPosX();
			result |= components[i].draw();
			if (i + 1 < components.size())
				ImGui::SameLine();

			avail_width -= ImGui::GetCursorPosX() - start_x;
		}

		return result;
	}
}
