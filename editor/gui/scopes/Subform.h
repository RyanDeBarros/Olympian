#pragma once

#include "gui/scopes/Form.h"
#include "gui/scopes/CollapsingSection.h"

#include <optional>

namespace oly::editor
{
	class Subform
	{
		Form::PauseImpl _pause;
		std::optional<CollapsingSection> _section;
		std::optional<Form> _subform;

	public:
		Subform(Form& form, const char* label, bool start_open = false);

		operator bool() const;
		Form& GetForm();
	};
}
