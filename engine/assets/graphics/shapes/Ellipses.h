#pragma once

#include "external/TOML.h"

#include "graphics/shapes/Ellipses.h"

namespace oly::assets
{
	extern rendering::Ellipse load_ellipse(TOMLNode node);
}
