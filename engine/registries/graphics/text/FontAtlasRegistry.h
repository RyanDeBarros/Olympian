#pragma once

#include "graphics/text/Font.h"

namespace oly::reg
{
	class FontAtlasRegistry
	{
		std::unordered_map<std::string, rendering::FontAtlasRes> font_atlases;

	public:
		void clear();

		rendering::FontAtlasRes load_font_atlas(const std::string& name, unsigned int index = 0);
		void free_font_atlas(const std::string& name);
	};
}
