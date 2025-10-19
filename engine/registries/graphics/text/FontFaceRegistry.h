#pragma once

#include "graphics/text/Font.h"
#include "core/util/ResourcePath.h"

namespace oly::reg
{
	class FontFaceRegistry
	{
		std::unordered_map<ResourcePath, rendering::FontFaceRef> font_faces;

	public:
		void clear();

		rendering::FontFaceRef load_font_face(const ResourcePath& file);
		void free_font_face(const ResourcePath& file);
	};
}
