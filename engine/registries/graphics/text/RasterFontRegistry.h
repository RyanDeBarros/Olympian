#pragma once

#include "graphics/text/RasterFont.h"

namespace oly::reg
{
	class RasterFontRegistry
	{
		std::unordered_map<std::string, rendering::RasterFontRef> raster_fonts;

	public:
		void clear();

		rendering::RasterFontRef load_raster_font(const std::string& file);
		void free_raster_font(const std::string& file);
	};
}
