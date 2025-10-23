#pragma once

#include "external/TOML.h"

#include "graphics/sprites/SpriteAtlas.h"

namespace oly::reg
{
	extern rendering::SpriteAtlas load_sprite_atlas(TOMLNode node);
}
