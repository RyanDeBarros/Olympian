#include "Fonts.h"

#include "registries/graphics/text/FontFaceRegistry.h"
#include "registries/graphics/text/FontAtlasRegistry.h"

namespace oly::context
{
	namespace internal
	{
		reg::FontFaceRegistry font_face_registry;
		reg::FontAtlasRegistry font_atlas_registry;
	}

	void internal::terminate_fonts()
	{
		internal::font_face_registry.clear();
		internal::font_atlas_registry.clear();
	}

	reg::FontFaceRegistry& font_face_registry()
	{
		return internal::font_face_registry;
	}

	reg::FontAtlasRegistry& font_atlas_registry()
	{
		return internal::font_atlas_registry;
	}

	rendering::FontFaceRef load_font_face(const std::string& file)
	{
		return internal::font_face_registry.load_font_face(file);
	}

	rendering::FontAtlasRef load_font_atlas(const std::string& file, unsigned int index)
	{
		return internal::font_atlas_registry.load_font_atlas(file, index);
	}
}
