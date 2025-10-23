#pragma once

#include "external/TOML.h"

#include "graphics/sprites/Sprite.h"

namespace oly::reg
{
	extern rendering::Sprite load_sprite(TOMLNode node);
}
