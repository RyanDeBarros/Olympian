#pragma once

#include "graphics/extensions/TileSet.h"

namespace oly::reg
{
	class TileSetRegistry
	{
		std::unordered_map<std::string, rendering::TileSetRes> tilesets;

	public:
		void clear();

		rendering::TileSetRes load_tileset(const std::string& file);
		void free_tileset(const std::string& file);
	};
}
