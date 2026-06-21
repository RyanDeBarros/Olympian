#pragma once

#include "gui/WidgetComponent.h"

namespace oly::editor::gui
{
	struct PropertyGrid
	{
		struct Key
		{
			static DrawResult GetDrawResult();

			// TODO v9.1 just SetText() - no components needed
			static void AddComponent(WidgetComponent component);
			static void SameLine();
		};

		struct Value
		{
			static DrawResult GetDrawResult();

			static void AddComponent(WidgetComponent component);
			static void SameLine();
		};

		struct Reset
		{
			static DrawResult GetDrawResult();

			static void AddComponent(WidgetComponent component);
			static void SameLine();
		};

		static void SubmitRow();
		static bool DirtyRow();

		static void Clear();
		static bool DirtyGrid();

		static bool BeginTable();
	};
}
