#include "WidgetComponentCommon.h"

namespace oly::editor::gui
{
	WidgetComponent VerticalSeparatorComponent()
	{
		WidgetComponent c;
		c.draw = []() { VerticalSeparator(); return false; };
		return c;
	}

	WidgetComponent TextComponent(const char* label)
	{
		WidgetComponent c;
		c.draw = [label]() { ImGui::TextUnformatted(label); return false; };
		return c;
	}
}
