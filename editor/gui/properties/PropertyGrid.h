#pragma once

#include "gui/WidgetComponent.h"

#include <string_view>

namespace oly::editor::gui
{
	struct PropertyGrid
	{
		PropertyGrid();
		PropertyGrid(const PropertyGrid&) = delete;
		PropertyGrid(PropertyGrid&&) noexcept;
		~PropertyGrid();
		PropertyGrid& operator=(PropertyGrid&&) = delete;

		struct Key
		{
			static void SetLabel(const std::string_view label);
		};

		struct Value
		{
			static DrawResult GetDrawResult();

			static void AddComponent(WidgetComponent component);
		};

		struct Reset
		{
			static void Button(size_t subrow = 0);
			static bool Activated(size_t subrow);
			static bool AnyActivated();
		};

		static void SubmitRow();
		static bool DirtyRow();

		static bool DirtyGrid();

		static bool BeginTable();
	};
}
