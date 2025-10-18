#pragma once

#include "core/types/SmartReference.h"
#include "graphics/text/TextElement.h"

namespace oly::reg
{
	class FontFaceRegistry;
	class FontAtlasRegistry;
	class RasterFontRegistry;
}

namespace oly::context
{
	namespace internal
	{
		extern void terminate_fonts();
	}

	extern reg::FontFaceRegistry& font_face_registry();
	extern reg::FontAtlasRegistry& font_atlas_registry();
	extern reg::RasterFontRegistry& raster_font_registry();

	extern rendering::FontFaceRef load_font_face(const std::string& file);
	extern rendering::FontAtlasRef load_font_atlas(const std::string& file, unsigned int index = 0);
	extern rendering::RasterFontRef load_raster_font(const std::string& file);
	extern rendering::TextElementFont load_font(const std::string& file, unsigned index = 0);
}
