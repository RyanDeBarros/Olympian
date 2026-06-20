#pragma once

#include "gui/WidgetComponent.h"

namespace oly::editor::gui
{
	struct PropertyGrid
	{
		enum Column
		{
			Key,
			Value,
			Reset,
			_C
		};

		static void SetColumn(Column column);
		static void SubmitRow();
		static DrawResult GetDrawResult(Column column);

		static void AddComponent(WidgetComponent component);
		static void SameLine();

		static bool DirtyValue();

		static void Clear();
		static bool DirtyGrid();
	};
}
