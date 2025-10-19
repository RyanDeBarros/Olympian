#pragma once

#include "graphics/text/RasterFont.h"
#include "core/util/ResourcePath.h"

namespace oly::reg
{
	class RasterFontRegistry
	{
		std::unordered_map<ResourcePath, rendering::RasterFontRef> raster_fonts;

	public:
		void clear();

		rendering::RasterFontRef load_raster_font(const ResourcePath& file);
		void free_raster_font(const ResourcePath& file);
	};
}
