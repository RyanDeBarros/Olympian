#pragma once

#include "WidgetComponent.h"

#include "gui/ImGuiWrapper.h"

namespace oly::editor::comp
{
	extern gui::WidgetComponent VerticalSeparator();
	extern gui::WidgetComponent Text(const char* label);

	template<typename T, typename... Args>
	extern gui::WidgetComponent InputData(const char* label, T& data, Args&&... args)
	{
		gui::WidgetComponent c;
		c.draw = [label, &data, ... args = std::forward<Args>(args)]() mutable  -> DrawResult {
			gui::IDScope scope(&data);
			return gui::InputData<T>{}(label, data, std::forward<Args>(args)...);
		};
		return c;
	}

	extern gui::WidgetComponent Generic(std::function<DrawResult()> draw);
}
