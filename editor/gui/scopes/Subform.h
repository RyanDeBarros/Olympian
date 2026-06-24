#pragma once

#include "gui/scopes/Form.h"
#include "gui/scopes/CollapsingSection.h"
#include "gui/properties/PropertyClipboard.h"

#include <optional>

namespace oly::editor
{
	class Subform
	{
		FormPause _pause;
		CollapsingSection _section;
		std::optional<Form> _subform;

	public:
		Subform(const char* label, const std::function<void(PropertyPage&)>& property_page_generator, bool start_open = false);
		Subform(const char* label, bool start_open = false);
		Subform(const Subform&) = delete;
		Subform(Subform&&) = delete;

		operator bool() const;
	};
}
