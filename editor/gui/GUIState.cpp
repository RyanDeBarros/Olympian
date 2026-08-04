#include "GUIState.h"

namespace oly::editor
{
	imtk::style_substack GUIState::InputDataStyleSubstack()
	{
		return imtk::style_substack(input_data_styles);
	}
}
