#include "Overlays.h"

#include "core/Colors.h"

#include <utility>

namespace oly::editor::gui
{
	void Overlay::QuadError(ImVec2 rect_start, ImVec2 rect_end)
	{
		ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_end, Color::MissingResource);
	}

	void Overlay::QuadWarning(ImVec2 rect_start, ImVec2 rect_end)
	{
		static const float MAX_BORDER_WIDTH = 10.f;
		static const int BAR_COUNT = 5;
		static const int BAR_MULTIPLIER = 2 * BAR_COUNT - 1;
		static const float THRESHOLD_WIDTH = MAX_BORDER_WIDTH * BAR_MULTIPLIER;

		const ImVec2 size = rect_end - rect_start;
		const float min_dimension = std::min(size.x, size.y);

		float border_width = MAX_BORDER_WIDTH;
		if (min_dimension < THRESHOLD_WIDTH)
			border_width = min_dimension / BAR_MULTIPLIER;
		
		float x_gap_width = MAX_BORDER_WIDTH;
		if (size.x > THRESHOLD_WIDTH)
			x_gap_width = (size.x - (BAR_COUNT * border_width)) / (BAR_COUNT - 1);

		float y_gap_width = MAX_BORDER_WIDTH;
		if (size.y > THRESHOLD_WIDTH)
			y_gap_width = (size.y - (BAR_COUNT * border_width)) / (BAR_COUNT - 1);

		// Vertical bars
		ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_start + ImVec2(border_width, size.y), Color::MissingResource);
		ImGui::GetWindowDrawList()->AddRectFilled(rect_end - ImVec2(border_width, size.y), rect_end, Color::MissingResource);
		for (int i = 1; i < BAR_COUNT - 1; ++i)
		{
			ImVec2 bar_start = rect_start + ImVec2(i * (border_width + x_gap_width), 0);
			const ImVec2 p1 = bar_start + ImVec2(0.4f * border_width, size.y);
			const ImVec2 p2 = bar_start + ImVec2(0.f, 0.f);
			const ImVec2 p3 = bar_start + ImVec2(border_width, 0.f);
			const ImVec2 p4 = bar_start + ImVec2(0.6f * border_width, size.y);

			ImGui::GetWindowDrawList()->AddTriangleFilled(p1, p2, p3, Color::MissingResource);
			ImGui::GetWindowDrawList()->AddTriangleFilled(p1, p3, p4, Color::MissingResource);
		}

		// Horizontal bars
		ImGui::GetWindowDrawList()->AddRectFilled(rect_start, rect_start + ImVec2(size.x, border_width), Color::MissingResource);
		ImGui::GetWindowDrawList()->AddRectFilled(rect_end - ImVec2(size.x, border_width), rect_end, Color::MissingResource);
		for (int i = 1; i < BAR_COUNT - 1; ++i)
		{
			ImVec2 bar_start = rect_start + ImVec2(0, i * (border_width + y_gap_width));
			const ImVec2 p1 = bar_start + ImVec2(0.f, 0.4f * border_width);
			const ImVec2 p2 = bar_start + ImVec2(size.x, 0.f);
			const ImVec2 p3 = bar_start + ImVec2(size.x, border_width);
			const ImVec2 p4 = bar_start + ImVec2(0.f, 0.6f * border_width);

			ImGui::GetWindowDrawList()->AddTriangleFilled(p1, p2, p3, Color::MissingResource);
			ImGui::GetWindowDrawList()->AddTriangleFilled(p1, p3, p4, Color::MissingResource);
		}
	}
}
