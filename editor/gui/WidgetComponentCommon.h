#pragma once

#include "WidgetComponent.h"

#include "ImGuiWrapper.h"

namespace oly::editor::gui
{
	extern WidgetComponent VerticalSeparatorComponent();
	extern WidgetComponent TextComponent(const char* label);

	template<typename T>
	extern WidgetComponent InputDataComponent(const char* label, T& data)
	{
		WidgetComponent c;
		c.draw = [label, &data]() { return InputData<float>{}(label, data); };
		return c;
	}
}
