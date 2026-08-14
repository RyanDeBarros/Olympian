#pragma once

#include "gui/WidgetComponent.h"

#include <imgui.h>

#include <span>

namespace oly::editor::gui
{
	struct InlineWidget
	{
		static imtk::item_result Draw(const std::span<WidgetComponent> components);
	};
}
