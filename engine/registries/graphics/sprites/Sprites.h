#pragma once

#include "external/TOML.h"

#include "graphics/sprites/Sprite.h"
#include "core/types/Variant.h"

namespace oly::reg
{
	namespace params
	{
		struct Sprite
		{
			Transform2D local;
			std::optional<std::string> texture;
			unsigned int texture_index = 0;
			std::optional<glm::vec4> modulation;
			std::optional<math::UVRect> tex_coords;

			struct SingleFrameFormat
			{
				GLuint frame = 0;
			};

			struct AutoFrameFormat
			{
				float speed = 1.0f;
				GLuint starting_frame = 0;
			};
			
			std::optional<Variant<SingleFrameFormat, AutoFrameFormat, graphics::AnimFrameFormat>> frame_format;
			std::optional<std::variant<ShearTransformModifier2D, PivotTransformModifier2D, OffsetTransformModifier2D>> modifier;
		};
	}

	extern params::Sprite sprite_params(TOMLNode node);
	extern rendering::Sprite load_sprite(TOMLNode node);
	extern rendering::Sprite load_sprite(const params::Sprite& params);
}
