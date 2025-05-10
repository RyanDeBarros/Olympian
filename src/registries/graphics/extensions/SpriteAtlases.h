#pragma once

#include "external/TOML.h"

#include "graphics/extensions/SpriteAtlas.h"
#include "registries/graphics/primitives/Sprites.h"

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
				bool row_major, row_up;
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

	extern rendering::SpriteAtlasExtension load_sprite_atlas(const TOMLNode& node);
	extern rendering::SpriteAtlasExtension load_sprite_atlas(const params::SpriteAtlas& params);
}
