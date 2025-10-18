#include "Fonts.h"

#include "registries/graphics/text/FontFaceRegistry.h"
#include "registries/graphics/text/FontAtlasRegistry.h"
#include "registries/graphics/text/RasterFontRegistry.h"
#include "registries/MetaSplitter.h"

namespace oly::context
{
	namespace internal
	{
		reg::FontFaceRegistry font_face_registry;
		reg::FontAtlasRegistry font_atlas_registry;
		reg::RasterFontRegistry raster_font_registry;
	}

	void internal::terminate_fonts()
	{
		internal::font_face_registry.clear();
		internal::font_atlas_registry.clear();
		internal::raster_font_registry.clear();
	}

	reg::FontFaceRegistry& font_face_registry()
	{
		return internal::font_face_registry;
	}

	reg::FontAtlasRegistry& font_atlas_registry()
	{
		return internal::font_atlas_registry;
	}

	reg::RasterFontRegistry& raster_font_registry()
	{
		return internal::raster_font_registry;
	}

	rendering::FontFaceRef load_font_face(const std::string& file)
	{
		return internal::font_face_registry.load_font_face(file);
	}

	rendering::FontAtlasRef load_font_atlas(const std::string& file, unsigned int index)
	{
		return internal::font_atlas_registry.load_font_atlas(file, index);
	}

	rendering::RasterFontRef load_raster_font(const std::string& file)
	{
		return internal::raster_font_registry.load_raster_font(file);
	}

	rendering::TextElementFont load_font(const std::string& file, unsigned index)
	{
		if (file.ends_with(".oly"))
		{
			// TODO v5 when font style is implemented, do check here for meta type
			//if (reg::MetaSplitter::meta(file).has_type("raster_font"))
			return load_raster_font(file);
		}
		else
			return load_font_atlas(file, index);
	}
}
