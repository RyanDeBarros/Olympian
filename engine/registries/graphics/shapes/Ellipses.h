#pragma once

#include "external/TOML.h"

#include "graphics/shapes/Ellipses.h"

namespace oly::reg
{
	extern rendering::Ellipse load_ellipse(TOMLNode node);
}
