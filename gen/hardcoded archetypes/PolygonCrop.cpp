#include "PolygonCrop.h"

namespace oly::gen
{
	PolygonCrop::PolygonCrop() :
		pentagon1(reg::load_polygon(context::load_toml("assets/polygonals/pentagon1.toml")["polygon"])),
		pentagon2(reg::load_polygon(context::load_toml("assets/polygonals/pentagon2.toml")["polygon"])),
		bordered_quad(reg::load_poly_composite(context::load_toml("assets/polygonals/bordered quad.toml")["poly_composite"]))
	{}

	void PolygonCrop::draw(bool flush_polygons) const
	{
		pentagon1.draw();
		pentagon2.draw();
		bordered_quad.draw();
		if (flush_polygons)
			context::render_polygons();
	}
}
