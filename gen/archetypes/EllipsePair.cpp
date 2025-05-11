#include "EllipsePair.h"

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
			reg::params::Ellipse ellipse1;
			reg::params::Ellipse ellipse2;

			Constructor();
		};

		Constructor::Constructor()
		{
		transformer.local.position = { (float)500, (float)0 };
			ellipse1.local.position = { (float)-300, (float)0 };
			ellipse1.local.scale = { (float)150, (float)150 };
			ellipse1.dimension.width = (float)2;
			ellipse1.dimension.height = (float)1;
			ellipse1.dimension.border = (float)0.3;
			ellipse1.color.fill_inner = { (float)1.0, (float)0.9, (float)0.8, (float)0.5 };
			ellipse1.color.fill_outer = { (float)1.0, (float)0.6, (float)0.2, (float)1.0 };
			ellipse1.color.border_inner = { (float)0.2, (float)0.6, (float)1.0, (float)1.0 };
			ellipse1.color.border_outer = { (float)0.8, (float)0.9, (float)1.0, (float)1.0 };

			ellipse2.local.scale = { (float)150, (float)150 };
			ellipse2.dimension.width = (float)1;
			ellipse2.dimension.height = (float)3;
			ellipse2.dimension.border = (float)0.4;
			ellipse2.dimension.border_exp = (float)2.0;
			ellipse2.dimension.fill_exp = (float)0.5;
			ellipse2.color.fill_inner = { (float)1.0, (float)0.9, (float)0.8, (float)0.5 };
			ellipse2.color.fill_outer = { (float)1.0, (float)0.6, (float)0.2, (float)1.0 };
			ellipse2.color.border_inner = { (float)0.0, (float)0.0, (float)0.0, (float)1.0 };
			ellipse2.color.border_outer = { (float)0.8, (float)0.9, (float)1.0, (float)0.0 };
		}

		static std::unique_ptr<Constructor> _c;
		static Constructor& constructor()
		{
			if (!_c)
				_c = std::make_unique<Constructor>();
			return *_c;
		}
	}

	void EllipsePair::free_constructor()
	{
		_c.reset();
	}

	EllipsePair::EllipsePair() :
		ellipse1(reg::load_ellipse(constructor().ellipse1)),
		ellipse2(reg::load_ellipse(constructor().ellipse2)),
		transformer(constructor().transformer.local, std::make_unique<TransformModifier2D>(*constructor().transformer.modifier))
	{
		ellipse1.transformer.attach_parent(&transformer);
		ellipse2.transformer.attach_parent(&transformer);
	}

	void EllipsePair::draw(bool flush_ellipses) const
	{
		ellipse1.draw();
		ellipse2.draw();
		if (flush_ellipses)
			context::render_ellipses();
	}

	void EllipsePair::on_tick() const
	{
	}
}
