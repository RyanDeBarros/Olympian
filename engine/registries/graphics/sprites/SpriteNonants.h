#pragma once

#include "external/TOML.h"

#include "graphics/sprites/SpriteNonant.h"

namespace oly::reg
{
	extern rendering::SpriteNonant load_sprite_nonant(TOMLNode node);
}
