#pragma once

#include "WidgetComponent.h"

#include "gui/ImGuiWrapper.h"
#include "gui/scopes/IDScope.h"

namespace oly::editor::gui
{
	extern WidgetComponent VerticalSeparatorComponent();
	extern WidgetComponent TextComponent(const char* label);

	template<typename T, typename... Args>
	extern WidgetComponent InputDataComponent(const char* label, T& data, Args&&... args)
	{
		WidgetComponent c;
		c.draw = [label, &data, ... args = std::forward<Args>(args)]() mutable { IDScope scope(&data); return InputData<T>{}(label, data, std::forward<Args>(args)...); };
		return c;
	}
}
