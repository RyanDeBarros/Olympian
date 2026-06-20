#pragma once

#include "gui/WidgetComponent.h"

#include <vector>

namespace oly::editor::gui
{
	class InlineWidget
	{
		std::vector<WidgetComponent> _components;

	public:
		DrawResult Draw();
		void AddComponent(WidgetComponent component);
	};
}
