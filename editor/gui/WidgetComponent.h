#pragma once

#include "gui/DrawResult.h"

#include <functional>

namespace oly::editor::gui
{
	struct WidgetComponent
	{
		std::function<DrawResult()> draw;
	};
}
