#include "PolygonCrop.h"

#include "registries/Loader.h"
#include "registries/graphics/primitives/Polygons.h"

namespace oly::gen
{
    PolygonCrop::PolygonCrop()
    {
        {
            reg::params::Transformer2D params;

            transformer = reg::load_transformer_2d(params);
        }

        {
            reg::params::Polygon params;
			params.local.position = { (float)0, (float)200 };
			params.local.scale = { (float)160, (float)160 };
			params.points.reserve(5);
			params.points.push_back({ (float)1, (float)-1 });
			params.points.push_back({ (float)1, (float)0 });
			params.points.push_back({ (float)0, (float)1 });
			params.points.push_back({ (float)-1, (float)0 });
			params.points.push_back({ (float)-1, (float)-1 });
			params.colors.reserve(5);
			params.colors.push_back({ (float)1.0, (float)1.0, (float)0.0, (float)1.0 });
			params.colors.push_back({ (float)1.0, (float)0.0, (float)1.0, (float)1.0 });
			params.colors.push_back({ (float)0.0, (float)1.0, (float)1.0, (float)1.0 });
			params.colors.push_back({ (float)0.0, (float)0.0, (float)0.0, (float)1.0 });
			params.colors.push_back({ (float)1.0, (float)1.0, (float)1.0, (float)1.0 });

            pentagon1.init(reg::load_polygon(std::move(params)));
        }

        {
            reg::params::Polygon params;
			params.local.position = { (float)-250, (float)0 };
			params.local.rotation = (float)-1;
			params.local.scale = { (float)320, (float)160 };
			params.points.reserve(5);
			params.points.push_back({ (float)1, (float)-1 });
			params.points.push_back({ (float)1, (float)0 });
			params.points.push_back({ (float)0, (float)1 });
			params.points.push_back({ (float)-1, (float)0 });
			params.points.push_back({ (float)-1, (float)-1 });
			params.colors.reserve(5);
			params.colors.push_back({ (float)1.0, (float)1.0, (float)0.0, (float)0.5 });
			params.colors.push_back({ (float)1.0, (float)0.0, (float)1.0, (float)0.5 });
			params.colors.push_back({ (float)0.0, (float)1.0, (float)1.0, (float)0.5 });
			params.colors.push_back({ (float)0.0, (float)0.0, (float)0.0, (float)0.5 });
			params.colors.push_back({ (float)1.0, (float)1.0, (float)1.0, (float)0.5 });

            pentagon2.init(reg::load_polygon(std::move(params)));
        }

        {
            reg::params::PolyComposite params;
			params.local.position = { (float)100, (float)-100 };
			params.local.scale = { (float)150, (float)150 };
			{
				reg::params::PolyComposite::BorderedNGonMethod _method;
				_method.ngon_base.points.reserve(4);
				_method.ngon_base.points.push_back({ (float)3, (float)-1 });
				_method.ngon_base.points.push_back({ (float)0, (float)2 });
				_method.ngon_base.points.push_back({ (float)-3, (float)-1 });
				_method.ngon_base.points.push_back({ (float)0, (float)0 });
				_method.ngon_base.fill_colors.reserve(1);
				_method.ngon_base.fill_colors.push_back({ (float)0.9, (float)0.9, (float)0.7, (float)1.0 });
				_method.ngon_base.border_colors.reserve(1);
				_method.ngon_base.border_colors.push_back({ (float)0.3, (float)0.15, (float)0.0, (float)1.0 });
				_method.ngon_base.border_width = (float)0.1;
				_method.ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;
				params.method = _method;
			}

            bordered_quad.init(reg::load_poly_composite(std::move(params)));
        }

		pentagon1->transformer.attach_parent(&transformer);
		pentagon2->transformer.attach_parent(&transformer);
		bordered_quad->transformer.attach_parent(&transformer);
    }


    void PolygonCrop::draw(bool flush_polygons) const
    {
		pentagon1->draw();
		pentagon2->draw();
		bordered_quad->draw();
		if (flush_polygons)
			context::render_polygons();
	}


    void PolygonCrop::on_tick() const
    {
	}
}
