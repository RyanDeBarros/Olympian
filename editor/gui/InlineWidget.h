#pragma once

#include "gui/DrawResult.h"
#include "gui/WidgetComponent.h"

#include <imgui.h>

#include <span>

namespace oly::editor::gui
{
	struct InlineWidget
	{
		static DrawResult Draw(const std::span<WidgetComponent> components);
	};
}
