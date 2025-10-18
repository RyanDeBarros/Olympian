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
			rendering::EllipseColorGradient color;
			rendering::EllipseDimension dimension;
		};
	}

	extern rendering::Ellipse load_ellipse(TOMLNode node);
	extern rendering::Ellipse load_ellipse(const params::Ellipse& params);
}
