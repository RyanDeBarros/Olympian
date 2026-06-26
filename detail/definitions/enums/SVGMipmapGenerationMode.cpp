#include "SVGMipmapGenerationMode.h"

namespace oly::detail
{
	std::ostream& operator<<(std::ostream& os, SVGMipmapGenerationMode mode)
	{
		os << "SVGMipmapGenerationMode(";

		switch (mode)
		{
		case SVGMipmapGenerationMode::Auto:
			os << "Auto";
			break;

		case SVGMipmapGenerationMode::Off:
			os << "Off";
			break;

		case SVGMipmapGenerationMode::Manual:
			os << "Manual";
			break;

		default:
			os << "unknown";
			break;
		}

		return os << ")";
	}
}
