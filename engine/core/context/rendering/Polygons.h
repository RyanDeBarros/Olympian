#pragma once

#include "external/TOML.h"

namespace oly::rendering
{
	class PolygonBatch;
}

namespace oly::context
{
	namespace internal
	{
		extern void init_polygons(const TOMLNode&);
		extern void terminate_polygons();
	}

	extern rendering::PolygonBatch& polygon_batch();
	extern void render_polygons();
}
