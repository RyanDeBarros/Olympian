#pragma once

#include "core/Types.h"

#include "gui/ImGuiWrapper.h"

namespace oly::editor::gui
{
	template<>
	struct InputData<Color4>
	{
		imtk::item_result operator()(const char* label, Color4& data) const;
	};
}
