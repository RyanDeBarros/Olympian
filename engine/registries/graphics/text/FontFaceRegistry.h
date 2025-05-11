#pragma once

#include "graphics/text/Font.h"

namespace oly::reg
{
	class FontFaceRegistry
	{
		std::unordered_map<std::string, rendering::FontFaceRes> font_faces;

	public:
		void clear();

		rendering::FontFaceRes load_font_face(const std::string& file);
		void free_font_face(const std::string& file);
	};
}
