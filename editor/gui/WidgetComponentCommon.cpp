#include "WidgetComponentCommon.h"

namespace oly::editor::comp
{
	gui::WidgetComponent VerticalSeparator()
	{
		gui::WidgetComponent c;
		c.draw = []() -> DrawResult { gui::VerticalSeparator(); return {}; };
		c.stretch = false;
		return c;
	}

	gui::WidgetComponent Text(const char* label)
	{
		gui::WidgetComponent c;
		c.draw = [label]() -> DrawResult { ImGui::TextUnformatted(label); return {}; };
		c.stretch = false;
		return c;
	}

	gui::WidgetComponent Generic(std::function<DrawResult()> draw, bool stretch)
	{
		gui::WidgetComponent c;
		c.draw = draw;
		c.stretch = stretch;
		return c;
	}
}
