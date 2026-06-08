#pragma once

#include "graphics/text/TextElement.h"

namespace oly::context
{
	namespace internal
	{
		extern void init_fonts();
	}

	extern rendering::FontFaceRef load_font_face(const detail::ResourcePath& file);
	extern rendering::FontAtlasRef load_font_atlas(const detail::ResourcePath& file, unsigned int index = 0);
	extern rendering::RasterFontRef load_raster_font(const detail::ResourcePath& file);
	extern rendering::FontFamilyRef load_font_family(const detail::ResourcePath& file);

	void free_font_face(const detail::ResourcePath& file);
	void free_font_atlas(const detail::ResourcePath& file, unsigned int index = 0);
	void free_raster_font(const detail::ResourcePath& file);
	void free_font_family(const detail::ResourcePath& file);

	extern rendering::FontSelection load_font_selection(const detail::ResourcePath& font_family, rendering::FontStyle style);
	extern rendering::Font load_font(const detail::ResourcePath& file, unsigned int index = 0);
}
