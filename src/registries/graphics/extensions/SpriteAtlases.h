#pragma once

#include "external/TOML.h"

#include "graphics/extensions/SpriteAtlas.h"

namespace oly::reg
{
	extern rendering::SpriteAtlasExtension load_sprite_atlas(const TOMLNode& node);
}
