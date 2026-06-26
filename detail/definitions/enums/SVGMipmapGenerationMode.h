#pragma once

#include <ostream>

namespace oly::detail
{
	enum class SVGMipmapGenerationMode
	{
		Auto = 0,
		Off,
		Manual
	};

	extern std::ostream& operator<<(std::ostream& os, SVGMipmapGenerationMode mode);
}
