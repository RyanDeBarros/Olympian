#pragma once

#include "gui/scopes/Form.h"
#include "gui/scopes/CollapsingSection.h"

#include <optional>

namespace oly::editor
{
	class Subform
	{
		FormPause _pause;
		std::optional<CollapsingSection> _section;
		std::optional<Form> _subform;

	public:
		Subform(const char* label, bool start_open = false);

		operator bool() const;
	};
}
