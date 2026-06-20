#pragma once

#include "core/Colors.h"

namespace oly::editor::gui
{
	class Outline
	{
		ImVec2 _start_pos;

	public:
		Outline();

		void Draw(ImU32 color, float border = 1.f) const;
	};
}
