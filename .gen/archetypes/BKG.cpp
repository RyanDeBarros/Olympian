#include "BKG.h"

#include "registries/Loader.h"
#include "registries/graphics/primitives/Polygons.h"

namespace oly::gen
{
    BKG::BKG()
    {
        {
            reg::params::Transformer2D params;

            transformer = reg::load_transformer_2d(params);
        }

        {
            reg::params::Polygon params;
			params.local.scale = { (float)1440, (float)1080 };
			params.points.reserve(4);
			params.points.push_back({ (float)-1, (float)-1 });
			params.points.push_back({ (float)1, (float)-1 });
			params.points.push_back({ (float)1, (float)1 });
			params.points.push_back({ (float)-1, (float)1 });
			params.colors.reserve(1);
			params.colors.push_back({ (float)0.2, (float)0.5, (float)0.8, (float)1.0 });

            bkg_rect.init(reg::load_polygon(std::move(params)));
        }

		bkg_rect->transformer.attach_parent(&transformer);
    }


    void BKG::draw(bool flush_polygons) const
    {
		bkg_rect->draw();
		if (flush_polygons)
			context::render_polygons();
	}


    void BKG::on_tick() const
    {
	}
}
