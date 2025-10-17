#pragma once

#include "core/types/SmartReference.h"

namespace oly::rendering
{
	class FontFace;
	class FontAtlas;
	class RasterFont;
}

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

	extern SmartReference<rendering::FontFace> load_font_face(const std::string& file);
	extern SmartReference<rendering::FontAtlas> load_font_atlas(const std::string& file, unsigned int index = 0);
	extern SmartReference<rendering::RasterFont> load_raster_font(const std::string& file);
}
