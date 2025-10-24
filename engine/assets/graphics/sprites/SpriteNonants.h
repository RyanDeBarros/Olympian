#pragma once

#include "external/TOML.h"

#include "graphics/sprites/SpriteNonant.h"

namespace oly::assets
{
	extern rendering::SpriteNonant load_sprite_nonant(TOMLNode node, const char* source = nullptr);
}
