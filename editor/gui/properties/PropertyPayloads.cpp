#include "PropertyPayloads.h"

namespace oly::editor::prop
{
	imtk::prop::payload PropertyPayloadInterface<std::string>::Dump(const std::string& value)
	{
		return imtk::prop::payload(value.data(), value.size(), imp::erase_type<std::string>());
	}

	std::optional<std::string> PropertyPayloadInterface<std::string>::Load(const imtk::prop::payload& payload)
	{
		if (payload.type == imp::erase_type<std::string>())
			return std::string(reinterpret_cast<const char*>(payload.data.data()), payload.data.size());
		else
			return std::nullopt;
	}
}
