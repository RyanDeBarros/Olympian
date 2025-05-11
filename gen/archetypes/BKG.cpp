#include "BKG.h"

namespace oly::gen
{
	BKG::Constructor::Constructor()
	{
		bkg_rect.local.scale = { (float)1440, (float)1080 };
		bkg_rect.points.reserve(4);
		bkg_rect.points.push_back({ (float)-1, (float)-1 });
		bkg_rect.points.push_back({ (float)1, (float)-1 });
		bkg_rect.points.push_back({ (float)1, (float)1 });
		bkg_rect.points.push_back({ (float)-1, (float)1 });
		bkg_rect.colors.reserve(1);
		bkg_rect.colors.push_back({ (float)0.2, (float)0.5, (float)0.8, (float)1.0 });
	}

	BKG::BKG(Constructor c) :
		bkg_rect(reg::load_polygon(c.bkg_rect)),
		transformer(c.transformer.local, std::make_unique<TransformModifier2D>(*c.transformer.modifier))
	{
		bkg_rect.transformer.attach_parent(&transformer);
	}

	void BKG::draw(bool flush_polygons) const
	{
		bkg_rect.draw();
		if (flush_polygons)
			context::render_polygons();
	}

	void BKG::on_tick() const
	{
	}
}
