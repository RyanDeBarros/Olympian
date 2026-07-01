#include "PositioningMode.h"

namespace oly::detail
{
	std::ostream& operator<<(std::ostream& os, PositioningMode mode)
	{
		os << "PositioningMode(";

		switch (mode)
		{
		case PositioningMode::Relative:
			os << "Relative";
			break;

		case PositioningMode::Absolute:
			os << "Absolute";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}
}
