#include "WidgetComponentCommon.h"

namespace oly::editor::comp
{
	gui::WidgetComponent Text(const char* label)
	{
		gui::WidgetComponent c;
		c.draw = [label]() -> imtk::item_result {
			ImGui::TextUnformatted(label);
			return {};
		};
		return c;
	}

	gui::WidgetComponent Generic(std::function<imtk::item_result()> draw)
	{
		gui::WidgetComponent c;
		c.draw = std::move(draw);
		return c;
	}
}
