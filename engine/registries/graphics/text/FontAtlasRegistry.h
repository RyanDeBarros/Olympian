#pragma once

#include "graphics/text/Font.h"

namespace oly::reg
{
	class FontAtlasRegistry
	{
		std::unordered_map<std::string, rendering::FontAtlasRef> font_atlases;

	public:
		void clear();

		rendering::FontAtlasRef load_font_atlas(const std::string& name, unsigned int index = 0);
		void free_font_atlas(const std::string& name);
	};
}
