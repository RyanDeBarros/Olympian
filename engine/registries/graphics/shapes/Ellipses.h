#pragma once

#include "external/TOML.h"

#include "graphics/shapes/Ellipses.h"

namespace oly::reg
{
	namespace params
	{
		struct Ellipse
		{
			Transform2D local;
			struct
			{
				std::optional<glm::vec4> border_inner, border_outer, fill_inner, fill_outer;
			} color;
			struct
			{
				std::optional<float> rx, ry, border, border_exp, fill_exp;
			} dimension;
		};
	}

	extern rendering::Ellipse load_ellipse(const TOMLNode& node);
	extern rendering::Ellipse load_ellipse(const params::Ellipse& params);
}
