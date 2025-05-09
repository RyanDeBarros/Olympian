#pragma once

#include "external/TOML.h"

#include "graphics/primitives/Polygons.h"

namespace oly::reg
{
	extern rendering::Polygon load_polygon(const TOMLNode& node);
	extern rendering::PolyComposite load_poly_composite(const TOMLNode& node);
	extern rendering::NGon load_ngon(const TOMLNode& node);
}
