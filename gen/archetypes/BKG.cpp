#include "BKG.h"

namespace oly::gen
{
	BKG::BKG() :
		bkg_rect(reg::load_polygon(context::load_toml("assets/polygonals/bkg rect.toml")["polygon"]))
	{}

	void BKG::draw(bool flush_polygons) const
	{
		bkg_rect.draw();
		if (flush_polygons)
			context::render_polygons();
	}
}
