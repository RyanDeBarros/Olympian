#pragma once

#include "external/TOML.h"

#include "graphics/sprites/SpriteNonant.h"
#include "registries/graphics/sprites/Sprites.h"

namespace oly::reg
{
	namespace params
	{
		struct SpriteNonant
		{
			Sprite sprite_params;
			math::Padding offsets;
			glm::vec2 nsize{};
		};
	}

	extern rendering::SpriteNonant load_sprite_nonant(TOMLNode node);
	extern rendering::SpriteNonant load_sprite_nonant(const params::SpriteNonant& params);
}
