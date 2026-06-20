#include "Subform.h"

namespace oly::editor
{
	Subform::Subform(const char* label, bool start_open)
		: _pause(),
		_section(_pause ? std::make_optional<CollapsingSection>(label, start_open) : std::nullopt),
		_subform(_pause ? std::make_optional<Form>() : std::nullopt)
	{
	}

	Subform::operator bool() const
	{
		return _pause && _section.has_value() && _section.value() && _subform.has_value() && _subform.value();
	}
}
