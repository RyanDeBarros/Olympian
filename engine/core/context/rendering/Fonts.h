#pragma once

#include "core/types/SmartReference.h"

namespace oly::rendering
{
	class FontFace;
	class FontAtlas;
}

namespace oly::reg
{
	class FontFaceRegistry;
	class FontAtlasRegistry;
}

namespace oly::context
{
	namespace internal
	{
		extern void terminate_fonts();
	}

	extern reg::FontFaceRegistry& font_face_registry();
	extern reg::FontAtlasRegistry& font_atlas_registry();

	extern SmartReference<rendering::FontFace> load_font_face(const std::string& file);
	extern SmartReference<rendering::FontAtlas> load_font_atlas(const std::string& file, unsigned int index = 0);
}
