#include "WidgetComponentCommon.h"

namespace oly::editor::comp
{
	gui::WidgetComponent Text(const char* label)
	{
		gui::WidgetComponent c;
		c.draw = [label]() -> DrawResult {
			ImGui::TextUnformatted(label);
			return {};
		};
		return c;
	}

	gui::WidgetComponent Generic(std::function<DrawResult()> draw)
	{
		gui::WidgetComponent c;
		c.draw = draw;
		return c;
	}
}
