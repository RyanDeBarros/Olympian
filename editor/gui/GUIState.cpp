#include "GUIState.h"

namespace oly::editor
{
	GUIState::InputDataStyleStack::InputDataStyleStack(InputDataStyleStack&& o) noexcept
		: _count(o._count)
	{
		o._count = 0;
	}

	GUIState::InputDataStyleStack::~InputDataStyleStack()
	{
		PopStyles();
	}
	
	void GUIState::InputDataStyleStack::PushStyle(gui::StyleCtorVariant ctor)
	{
		GUIState::input_data_styles.push_back(std::move(ctor));
		++_count;
	}

	void GUIState::InputDataStyleStack::PopStyles()
	{
		for (size_t i = 0; i < _count; ++i)
			GUIState::input_data_styles.pop_back();
		_count = 0;
	}
}
