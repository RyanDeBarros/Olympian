#pragma once

#include "graphics/text/Font.h"

namespace oly::reg
{
	class FontAtlasRegistry
	{
		struct FontAtlasKey
		{
			std::string file;
			unsigned int index;

			bool operator==(const FontAtlasKey&) const = default;
		};

		struct FontAtlasHash
		{
			size_t operator()(const FontAtlasKey& k) const { return std::hash<std::string>{}(k.file) ^ std::hash<unsigned int>{}(k.index); }
		};

		std::unordered_map<FontAtlasKey, rendering::FontAtlasRef, FontAtlasHash> font_atlases;

	public:
		void clear();

		rendering::FontAtlasRef load_font_atlas(const std::string& name, unsigned int index = 0);
		void free_font_atlas(const std::string& name, unsigned int index = 0);
	};
}
