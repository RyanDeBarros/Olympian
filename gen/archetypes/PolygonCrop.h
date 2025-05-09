#pragma once

#include "Olympian.h"

namespace oly::gen
{
	struct PolygonCrop
	{
		rendering::Polygon pentagon1, pentagon2;
		rendering::PolyComposite bordered_quad;

		PolygonCrop();

		void draw(bool flush_polygons) const;
	};
}
