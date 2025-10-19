#pragma once

#include "graphics/text/Font.h"

namespace oly::reg
{
	class FontAtlasRegistry
	{
		struct FontAtlasKey
		{
			ResourcePath file;
			unsigned int index;

			bool operator==(const FontAtlasKey&) const = default;
		};

		struct FontAtlasHash
		{
			size_t operator()(const FontAtlasKey& k) const { return std::hash<ResourcePath>{}(k.file) ^ std::hash<unsigned int>{}(k.index); }
		};

		std::unordered_map<FontAtlasKey, rendering::FontAtlasRef, FontAtlasHash> font_atlases;

	public:
		void clear();

		rendering::FontAtlasRef load_font_atlas(const ResourcePath& file, unsigned int index = 0);
		void free_font_atlas(const ResourcePath& file, unsigned int index = 0);
	};
}
