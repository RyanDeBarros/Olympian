#pragma once

#include "gui/scopes/Form.h"
#include "gui/scopes/CollapsingSection.h"

#include <optional>

namespace oly::editor
{
	class Subform
	{
		FormPause _pause;
		CollapsingSection _section;
		std::optional<Form> _subform;

	public:
		Subform(const char* label, const imtk::prop::view_generator& property_generator, bool start_open = false);
		Subform(const char* label, bool start_open = false);
		Subform(const Subform&) = delete;
		Subform(Subform&&) = delete;

		operator bool() const;
	};
}
