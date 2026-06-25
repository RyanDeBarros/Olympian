#pragma once

#include "gui/WidgetComponent.h"
#include "gui/properties/PropertyGroup.h"

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
			static DrawResult GetDrawResult();

			static void SetLabel(const std::string_view label);
		};

		struct Value
		{
			static DrawResult GetDrawResult();

			static void AddComponent(WidgetComponent component);
			static bool AppendView(std::unique_ptr<IPropertyView>&& prop);
		};

		struct Reset
		{
			static void Button(size_t subrow = 0);
			static bool Activated(size_t subrow);
			static bool AnyActivated();
		};

		static DrawResult GetFullDrawResult();

		static void SubmitRow();
		static bool DirtyRow();

		static bool DirtyGrid();

		static bool BeginTable();
	};
}
