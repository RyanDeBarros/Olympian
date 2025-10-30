#pragma once

#include "external/TOML.h"

#include "graphics/shapes/Polygons.h"
#include "core/types/Variant.h"

namespace oly::assets
{
	extern rendering::Polygon load_polygon(TOMLNode node, const char* source = nullptr);
	extern rendering::PolyComposite load_poly_composite(TOMLNode node, const char* source = nullptr);
	extern rendering::NGon load_ngon(TOMLNode node, const char* source = nullptr);
}
