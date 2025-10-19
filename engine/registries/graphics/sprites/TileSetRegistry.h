#pragma once

#include "graphics/sprites/TileSet.h"
#include "core/util/ResourcePath.h"

namespace oly::reg
{
	class TileSetRegistry
	{
		std::unordered_map<ResourcePath, rendering::TileSetRef> tilesets;

	public:
		void clear();

		rendering::TileSetRef load_tileset(const ResourcePath& file);
		void free_tileset(const ResourcePath& file);
	};
}
