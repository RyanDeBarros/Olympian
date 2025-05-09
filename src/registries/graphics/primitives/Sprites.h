#pragma once

#include "external/TOML.h"

#include "graphics/primitives/Sprites.h"

namespace oly::reg
{
	extern rendering::Sprite load_sprite(const TOMLNode& node);
}
