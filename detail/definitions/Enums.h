#pragma once

namespace oly::detail
{
	enum class StorageMode
	{
		Discard = 0,
		Keep
	};

	enum class SVGMipmapGenerationMode
	{
		Auto = 0,
		Off,
		Manual
	};
}
