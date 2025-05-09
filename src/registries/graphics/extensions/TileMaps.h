#pragma once

#include "external/TOML.h"

#include "graphics/extensions/TileMap.h"

namespace oly::reg
{
	extern rendering::TileMap load_tilemap(const TOMLNode& node);
}
