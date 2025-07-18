#pragma once

#include "graphics/extensions/TileSet.h"

namespace oly::reg
{
	class TileSetRegistry
	{
		std::unordered_map<std::string, rendering::TileSetRef> tilesets;

	public:
		void clear();

		rendering::TileSetRef load_tileset(const std::string& file);
		void free_tileset(const std::string& file);
	};
}
