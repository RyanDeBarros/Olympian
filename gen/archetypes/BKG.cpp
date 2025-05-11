#include "BKG.h"

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
			reg::params::Polygon bkg_rect;

			Constructor();
		};

		Constructor::Constructor()
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

		static std::unique_ptr<Constructor> _c;
		static Constructor& constructor()
		{
			if (!_c)
				_c = std::make_unique<Constructor>();
			return *_c;
		}
	}

	void BKG::free_constructor()
	{
		_c.reset();
	}

	BKG::BKG() :
		bkg_rect(reg::load_polygon(std::move(constructor().bkg_rect))),
		transformer(constructor().transformer.local, std::make_unique<TransformModifier2D>(*constructor().transformer.modifier))
	{
		bkg_rect.transformer.attach_parent(&transformer);
		free_constructor();
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
