#pragma once

#include "external/TOML.h"

#include "graphics/primitives/Sprites.h"

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
			std::optional<rendering::UVRect> tex_coords;

			struct SingleFrameFormat
			{
				GLuint frame = 0;
			};
			struct AutoFrameFormat
			{
				float speed = 1.0f;
				GLuint starting_frame = 0;
			};
			enum FrameFormatIndex
			{
				SINGLE,
				AUTO,
				CUSTOM
			};
			std::optional<std::variant<SingleFrameFormat, AutoFrameFormat, graphics::AnimFrameFormat>> frame_format;
			std::optional<std::variant<ShearTransformModifier2D, PivotTransformModifier2D, OffsetTransformModifier2D>> modifier;
		};
	}

	extern params::Sprite sprite_params(const TOMLNode& node);
	extern rendering::Sprite load_sprite(const TOMLNode& node);
	extern rendering::Sprite load_sprite(const params::Sprite& params);
}
