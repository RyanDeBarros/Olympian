#pragma once

#include "graphics/text/Font.h"

namespace oly::reg
{
	class FontFaceRegistry
	{
		std::unordered_map<std::string, rendering::FontFaceRef> font_faces;

	public:
		void clear();

		rendering::FontFaceRef load_font_face(const std::string& file);
		void free_font_face(const std::string& file);
	};
}
