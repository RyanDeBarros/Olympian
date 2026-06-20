#pragma once

#include <vector>
#include <functional>

namespace oly::editor::gui
{
	struct InlineWidgetSettings
	{
	};

	class InlineWidget
	{
		std::vector<std::function<void()>> _components;

	public:
		void Draw(InlineWidgetSettings settings = {});
		void AddComponent(std::function<void()> draw_fn);
	};
}
