#pragma once

#include "graphics/text/TextElement.h"

namespace oly::reg
{
	class FontFaceRegistry;
	class FontAtlasRegistry;
	class RasterFontRegistry;
	class FontFamilyRegistry;
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
	extern reg::FontFamilyRegistry& font_family_registry();

	extern rendering::FontFaceRef load_font_face(const ResourcePath& file);
	extern rendering::FontAtlasRef load_font_atlas(const ResourcePath& file, unsigned int index = 0);
	extern rendering::RasterFontRef load_raster_font(const ResourcePath& file);
	extern rendering::FontFamilyRef load_font_family(const ResourcePath& file);
	extern rendering::FontSelection load_font_selection(const ResourcePath& font_family, rendering::FontStyle style);
	extern rendering::Font load_font(const ResourcePath& file, unsigned int index = 0);
}
