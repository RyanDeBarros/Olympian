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

			struct
			{
				float x_left = 0.0f, x_right = 0.0f, y_bottom = 0.0f, y_top = 0.0f;
			} offsets;

			glm::vec2 nsize{};
		};
	}

	extern rendering::SpriteNonant load_sprite_nonant(const TOMLNode& node);
	extern rendering::SpriteNonant load_sprite_nonant(const params::SpriteNonant& params);
}
