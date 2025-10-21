#pragma once

#include "external/TOML.h"

#include "graphics/sprites/SpriteAtlas.h"
#include "registries/graphics/sprites/Sprites.h"
#include "core/types/Variant.h"

namespace oly::reg
{
	namespace params
	{
		struct SpriteAtlas
		{
			Sprite sprite_params;

			struct Frame
			{
				GLuint rows, cols;
				float delay_seconds;
				bool row_major = true, row_up = true;
			};
			struct StaticFrame
			{
				GLuint frame;
			};
			std::optional<Variant<Frame, StaticFrame>> frame;

			GLuint starting_frame = 0;
			float starting_time = 0.0f;
		};
	}

	extern rendering::SpriteAtlas load_sprite_atlas(TOMLNode node);
	extern rendering::SpriteAtlas load_sprite_atlas(const params::SpriteAtlas& params);
}
