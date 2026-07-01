#include "Subform.h"

#include "gui/properties/PropertyGroup.h"

namespace oly::editor
{
	// TODO DEBT use generators for as many headers as possible
	Subform::Subform(const char* label, const std::function<void(PropertyPage&)>& property_page_generator, bool start_open)
		: _pause(), _section(label, start_open)
	{
		PropertyGroup::CheckHeader([&property_page_generator]() -> PropertyPage {
			PropertyPage props;
			property_page_generator(props);
			return props;
		});

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
