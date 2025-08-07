#include "EllipsePair.h"

#include "registries/Loader.h"
#include "registries/graphics/primitives/Ellipses.h"

namespace oly::gen
{
    EllipsePair::EllipsePair()
    {
        {
            reg::params::Transformer2D params;
			params.local.position = { (float)500, (float)0 };

            transformer = reg::load_transformer_2d(params);
        }

        {
            reg::params::Ellipse params;
			params.local.position = { (float)-300, (float)0 };
			params.local.scale = { (float)150, (float)150 };
			params.dimension.rx = (float)2;
			params.dimension.ry = (float)1;
			params.dimension.border = (float)0.3;
			params.color.fill_inner = { (float)1.0, (float)0.9, (float)0.8, (float)0.5 };
			params.color.fill_outer = { (float)1.0, (float)0.6, (float)0.2, (float)1.0 };
			params.color.border_inner = { (float)0.2, (float)0.6, (float)1.0, (float)1.0 };
			params.color.border_outer = { (float)0.8, (float)0.9, (float)1.0, (float)1.0 };

            ellipse1.init(reg::load_ellipse(params));
        }

        {
            reg::params::Ellipse params;
			params.local.scale = { (float)150, (float)150 };
			params.dimension.rx = (float)1;
			params.dimension.ry = (float)3;
			params.dimension.border = (float)0.4;
			params.dimension.border_exp = (float)2.0;
			params.dimension.fill_exp = (float)0.5;
			params.color.fill_inner = { (float)1.0, (float)0.9, (float)0.8, (float)0.5 };
			params.color.fill_outer = { (float)1.0, (float)0.6, (float)0.2, (float)1.0 };
			params.color.border_inner = { (float)0.0, (float)0.0, (float)0.0, (float)1.0 };
			params.color.border_outer = { (float)0.8, (float)0.9, (float)1.0, (float)0.0 };

            ellipse2.init(reg::load_ellipse(params));
        }

		ellipse1->transformer.attach_parent(&transformer);
		ellipse2->transformer.attach_parent(&transformer);
    }


    void EllipsePair::draw(bool flush_ellipses) const
    {
		ellipse1->draw();
		ellipse2->draw();
		if (flush_ellipses)
			context::render_ellipses();
	}


    void EllipsePair::on_tick() const
    {
	}
}
