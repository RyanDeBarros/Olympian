#include "InlineWidget.h"

#include <imgui.h>

namespace oly::editor::gui
{
	DrawResult InlineWidget(const std::span<WidgetComponent> components)
	{
		DrawResult result;

		// TODO v9.1 don't use SameLine() in DrawDynamicList() -> use inner table again.
		for (size_t i = 0; i < components.size(); ++i)
		{
			result |= components[i].draw();

			if (i + 1 < components.size())
				ImGui::SameLine();
		}

		return result;
	}
}
