#pragma once

#include "gui/WidgetComponent.h"

#include <imtk.hpp>

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

		operator bool() const;

		struct Key
		{
			static void SetLabel(const std::string_view label);
		};

		struct Value
		{
			static imtk::item_result GetDrawResult();

			static void AddComponent(WidgetComponent component);
			static bool CheckProperty(std::unique_ptr<imtk::prop::iview> prop);
		};

		struct Reset
		{
			static void Button(size_t subrow = 0);
			static bool Activated(size_t subrow);
			static bool AnyActivated();
		};

		static imtk::item_result GetFullDrawResult();

		static void SubmitRow();
		static bool DirtyRow();
		
		static bool DirtyGrid();
		static bool CheckHeader(const imtk::prop::view_generator& generator);

		static bool BeginForm(ImGuiID id);
		static void EndForm(bool table_visible);
	};
}
