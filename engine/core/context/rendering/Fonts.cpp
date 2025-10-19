#include "Fonts.h"

#include "registries/graphics/text/FontFaceRegistry.h"
#include "registries/graphics/text/FontAtlasRegistry.h"
#include "registries/graphics/text/RasterFontRegistry.h"
#include "registries/graphics/text/FontFamilyRegistry.h"
#include "registries/MetaSplitter.h"

#include "core/base/Errors.h"
#include "core/util/LoggerOperators.h"

namespace oly::context
{
	namespace internal
	{
		reg::FontFaceRegistry font_face_registry;
		reg::FontAtlasRegistry font_atlas_registry;
		reg::RasterFontRegistry raster_font_registry;
		reg::FontFamilyRegistry font_family_registry;
	}

	void internal::terminate_fonts()
	{
		internal::font_face_registry.clear();
		internal::font_atlas_registry.clear();
		internal::raster_font_registry.clear();
		internal::font_family_registry.clear();
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

	reg::FontFamilyRegistry& font_family_registry()
	{
		return internal::font_family_registry;
	}

	rendering::FontFaceRef load_font_face(const ResourcePath& file)
	{
		return internal::font_face_registry.load_font_face(file);
	}

	rendering::FontAtlasRef load_font_atlas(const ResourcePath& file, unsigned int index)
	{
		return internal::font_atlas_registry.load_font_atlas(file, index);
	}

	rendering::RasterFontRef load_raster_font(const ResourcePath& file)
	{
		return internal::raster_font_registry.load_raster_font(file);
	}

	rendering::FontFamilyRef load_font_family(const ResourcePath& file)
	{
		return internal::font_family_registry.load_font_family(file);
	}

	rendering::FontSelection load_font_selection(const ResourcePath& font_family, rendering::FontStyle style)
	{
		rendering::FontSelection font{ .family = load_font_family(font_family), .style = style };
		if (!font.style_exists())
			OLY_LOG_WARNING(true, "CONTEXT") << LOG.source_info.full_source() << "Font style (" << (unsigned int)style << ") not supported by family " << font_family << LOG.nl;
		return font;
	}

	rendering::Font load_font(const ResourcePath& file, unsigned int index)
	{
		if (file.is_import_path())
		{
			auto meta = reg::MetaSplitter::meta(file);
			if (meta.has_type("raster_font"))
				return load_raster_font(file);
			else if (meta.has_type("font_family"))
				return load_font_selection(file, index);
			else
			{
				std::optional<std::string> type = meta.get_type();
				OLY_LOG_ERROR(true, "CONTEXT") << LOG.source_info.full_source() << file << " has unrecognized meta type: \"" << (type ? *type : "") << "\"" << LOG.nl;
				throw Error(ErrorCode::LOAD_ASSET);
			}
		}
		else
			return load_font_atlas(file, index);
	}
}
