#pragma once

#include <imtk.hpp>

namespace oly::editor
{
	struct GUIState
	{
		static inline imtk::style_stack input_data_styles;

		static imtk::style_substack InputDataStyleSubstack();
	};
}
