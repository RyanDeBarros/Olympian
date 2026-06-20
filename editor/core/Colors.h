#pragma once

#include <imgui.h>

namespace oly::editor
{
	struct Color
	{
		enum : ImU32
		{
			Azure = IM_COL32(0, 127, 255, 255),
			Black = IM_COL32(0, 0, 0, 255),
			Blue = IM_COL32(0, 0, 255, 255),
			Green = IM_COL32(0, 255, 0, 255),
			Magenta = IM_COL32(255, 0, 255, 255),
			Red = IM_COL32(255, 0, 0, 255),
			White = IM_COL32(255, 255, 255, 255),
			Yellow = IM_COL32(255, 255, 0, 255),

			Error = Red,
			Success = Green,
			Warning = Yellow,
			MissingResource = Magenta,
		};

		constexpr static ImU32 Grey(unsigned char level)
		{
			return IM_COL32(level, level, level, 255);
		}
	};
}
