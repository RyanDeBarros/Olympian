#pragma once

#include "external/TOML.h"

#include "graphics/sprites/TileMap.h"

namespace oly::reg
{
	extern rendering::TileMap load_tilemap(TOMLNode node);
}
