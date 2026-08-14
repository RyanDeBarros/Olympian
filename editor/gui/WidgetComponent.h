#pragma once

#include <imtk.hpp>

namespace oly::editor::gui
{
	struct WidgetComponent
	{
		std::function<imtk::item_result()> draw;

		explicit WidgetComponent() = default;
	};
}
