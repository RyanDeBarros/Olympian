#include "Subform.h"

namespace oly::editor
{
	Subform::Subform(Form& form, const char* label, bool start_open)
		: _pause(form.Pause()),
		_section(_pause ? std::make_optional<CollapsingSection>(label, start_open) : std::nullopt),
		_subform(_pause ? std::make_optional<Form>() : std::nullopt)
	{
	}

	Subform::operator bool() const
	{
		return _pause && _section.has_value() && _section.value() && _subform.has_value() && _subform.value();
	}

	Form& Subform::GetForm()
	{
		return _subform.value();
	}
}
