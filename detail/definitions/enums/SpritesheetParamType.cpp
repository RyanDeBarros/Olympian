#include "SpritesheetParamType.h"

namespace oly::detail
{
	std::ostream& operator<<(std::ostream& os, SpritesheetParamType type)
	{
		os << "SpritesheetParamType(";

		switch (type)
		{
		case SpritesheetParamType::Index:
			os << "Index";
			break;

		case SpritesheetParamType::Pixel:
			os << "Pixel";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}
}
