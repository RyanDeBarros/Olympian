#pragma once

#include "external/TOML.h"

#include "graphics/sprites/Sprite.h"

namespace oly::assets
{
	extern rendering::Sprite load_sprite(TOMLNode node, const char* source = nullptr);
}
