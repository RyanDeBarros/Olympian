#pragma once

#include "gui/DrawResult.h"
#include "gui/WidgetComponent.h"

#include <span>

namespace oly::editor::gui
{
	extern DrawResult InlineWidget(const std::span<WidgetComponent> components);
}
