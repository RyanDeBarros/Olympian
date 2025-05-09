#pragma once

#include "external/TOML.h"

#include "graphics/primitives/Ellipses.h"

namespace oly::reg
{
	extern rendering::Ellipse load_ellipse(const TOMLNode& node);
}
