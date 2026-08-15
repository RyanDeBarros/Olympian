#include "Subform.h"

namespace oly::editor
{
	// TODO DEBT use generators for as many headers as possible
	Subform::Subform(const char* label, const imtk::prop::view_generator& property_generator, bool start_open)
		: _pause(), _section(label, start_open)
	{
		imtk::prop::grid::check_header(property_generator);

		if (_section)
			_subform.emplace(Form());
	}

	Subform::Subform(const char* label, bool start_open)
		: _pause(), _section(label, start_open), _subform(_section ? std::make_optional<Form>() : std::nullopt)
	{
	}

	Subform::operator bool() const
	{
		return _pause && _section && _subform && *_subform;
	}
}
