#pragma once

#include "external/TOML.h"

#include "graphics/sprites/TileMap.h"

namespace oly::reg
{
	namespace params
	{
		struct TileMap
		{
			Transform2D local;
			struct Layer
			{
				std::string tileset;
				std::vector<glm::ivec2> tiles;
				std::optional<size_t> z;
			};
			std::vector<Layer> layers;
		};
	}

	extern rendering::TileMap load_tilemap(TOMLNode node);
	extern rendering::TileMap load_tilemap(const params::TileMap& params);
}
