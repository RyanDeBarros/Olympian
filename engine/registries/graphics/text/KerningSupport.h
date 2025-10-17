#pragma once

#include "external/TOML.h"

#include "graphics/text/Kerning.h"

namespace oly::reg
{
	extern rendering::Kerning parse_kerning(const TOMLNode& node);
}
