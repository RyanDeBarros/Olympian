#pragma once

#include "gui/DrawResult.h"

#include <functional>

namespace oly::editor::gui
{
	// TODO v9.1 virtual DrawResult Draw() method instead, and just define a GenericWidgetComponent that uses std::function?
	struct WidgetComponent
	{
		std::function<DrawResult()> draw;
	};
}
