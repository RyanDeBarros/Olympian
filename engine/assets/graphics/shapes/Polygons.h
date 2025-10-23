#pragma once

#include "external/TOML.h"

#include "graphics/shapes/Polygons.h"
#include "core/types/Variant.h"

namespace oly::assets
{
	extern rendering::Polygon load_polygon(TOMLNode node);
	extern rendering::PolyComposite load_poly_composite(TOMLNode node);
	extern rendering::NGon load_ngon(TOMLNode node);
}
