#pragma once

#include "external/TOML.h"

#include "graphics/sprites/TileMap.h"

namespace oly::assets
{
	extern rendering::TileMap load_tilemap(TOMLNode node, const char* source = nullptr);
}
