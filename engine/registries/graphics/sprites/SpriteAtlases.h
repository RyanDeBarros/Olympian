#pragma once

#include "external/TOML.h"

#include "graphics/sprites/SpriteAtlas.h"
#include "registries/graphics/sprites/Sprites.h"

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
			std::optional<std::variant<Frame, StaticFrame>> frame;

			std::optional<GLuint> starting_frame;
			std::optional<float> starting_time;
		};
	}

	extern rendering::SpriteAtlas load_sprite_atlas(const TOMLNode& node);
	extern rendering::SpriteAtlas load_sprite_atlas(const params::SpriteAtlas& params);
}
