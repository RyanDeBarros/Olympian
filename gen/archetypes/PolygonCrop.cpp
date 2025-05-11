#include "PolygonCrop.h"

namespace oly::gen
{
	namespace
	{
		struct Constructor
		{
			struct
			{
				Transform2D local;
				std::unique_ptr<TransformModifier2D> modifier;
			} transformer;
			reg::params::Polygon pentagon1;
			reg::params::Polygon pentagon2;
			reg::params::PolyComposite bordered_quad;

			Constructor();
		};

		Constructor::Constructor()
		{
			pentagon1.local.position = { (float)0, (float)200 };
			pentagon1.local.scale = { (float)160, (float)160 };
			pentagon1.points.reserve(5);
			pentagon1.points.push_back({ (float)1, (float)-1 });
			pentagon1.points.push_back({ (float)1, (float)0 });
			pentagon1.points.push_back({ (float)0, (float)1 });
			pentagon1.points.push_back({ (float)-1, (float)0 });
			pentagon1.points.push_back({ (float)-1, (float)-1 });
			pentagon1.colors.reserve(5);
			pentagon1.colors.push_back({ (float)1.0, (float)1.0, (float)0.0, (float)1.0 });
			pentagon1.colors.push_back({ (float)1.0, (float)0.0, (float)1.0, (float)1.0 });
			pentagon1.colors.push_back({ (float)0.0, (float)1.0, (float)1.0, (float)1.0 });
			pentagon1.colors.push_back({ (float)0.0, (float)0.0, (float)0.0, (float)1.0 });
			pentagon1.colors.push_back({ (float)1.0, (float)1.0, (float)1.0, (float)1.0 });

			pentagon2.local.position = { (float)-250, (float)0 };
			pentagon2.local.rotation = (float)-1;
			pentagon2.local.scale = { (float)320, (float)160 };
			pentagon2.points.reserve(5);
			pentagon2.points.push_back({ (float)1, (float)-1 });
			pentagon2.points.push_back({ (float)1, (float)0 });
			pentagon2.points.push_back({ (float)0, (float)1 });
			pentagon2.points.push_back({ (float)-1, (float)0 });
			pentagon2.points.push_back({ (float)-1, (float)-1 });
			pentagon2.colors.reserve(5);
			pentagon2.colors.push_back({ (float)1.0, (float)1.0, (float)0.0, (float)0.5 });
			pentagon2.colors.push_back({ (float)1.0, (float)0.0, (float)1.0, (float)0.5 });
			pentagon2.colors.push_back({ (float)0.0, (float)1.0, (float)1.0, (float)0.5 });
			pentagon2.colors.push_back({ (float)0.0, (float)0.0, (float)0.0, (float)0.5 });
			pentagon2.colors.push_back({ (float)1.0, (float)1.0, (float)1.0, (float)0.5 });

			bordered_quad.local.position = { (float)100, (float)-100 };
			bordered_quad.local.scale = { (float)150, (float)150 };
			{
				reg::params::PolyComposite::BorderedNGonMethod method;
				method.ngon_base.points.reserve(4);
				method.ngon_base.points.push_back({ (float)3, (float)-1 });
				method.ngon_base.points.push_back({ (float)0, (float)2 });
				method.ngon_base.points.push_back({ (float)-3, (float)-1 });
				method.ngon_base.points.push_back({ (float)0, (float)0 });
				method.ngon_base.fill_colors.reserve(1);
				method.ngon_base.fill_colors.push_back({ (float)0.9, (float)0.9, (float)0.7, (float)1.0 });
				method.ngon_base.border_colors.reserve(1);
				method.ngon_base.border_colors.push_back({ (float)0.3, (float)0.15, (float)0.0, (float)1.0 });
				method.ngon_base.border_width = (float)0.1;
				method.ngon_base.border_pivot = cmath::BorderPivot::MIDDLE;
				bordered_quad.method = method;
			}
		}

		static std::unique_ptr<Constructor> _c;
		static Constructor& constructor()
		{
			if (!_c)
				_c = std::make_unique<Constructor>();
			return *_c;
		}
	}

	void PolygonCrop::free_constructor()
	{
		_c.reset();
	}

	PolygonCrop::PolygonCrop() :
		pentagon1(reg::load_polygon(constructor().pentagon1)),
		pentagon2(reg::load_polygon(constructor().pentagon2)),
		bordered_quad(reg::load_poly_composite(constructor().bordered_quad)),
		transformer(constructor().transformer.local, std::make_unique<TransformModifier2D>(*constructor().transformer.modifier))
	{
		pentagon1.transformer.attach_parent(&transformer);
		pentagon2.transformer.attach_parent(&transformer);
		bordered_quad.transformer.attach_parent(&transformer);
	}

	void PolygonCrop::draw(bool flush_polygons) const
	{
		pentagon1.draw();
		pentagon2.draw();
		bordered_quad.draw();
		if (flush_polygons)
			context::render_polygons();
	}

	void PolygonCrop::on_tick() const
	{
	}
}
