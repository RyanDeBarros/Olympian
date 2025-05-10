#include "EllipsePair.h"

namespace oly::gen
{
	EllipsePair::Constructor::Constructor()
	{
		ellipse1.local = {
			{ -300.0f, 0.0f },
			{},
			{ 150.0f, 150.0f }
		};
		ellipse1.dimension.width = 2;
		ellipse1.dimension.height = 1;
		ellipse1.dimension.border = 0.3f;
		ellipse1.color.fill_inner = { 1.0f, 0.9f, 0.8f, 0.5f };
		ellipse1.color.fill_outer = { 1.0f, 0.6f, 0.2f, 1.0f };
		ellipse1.color.border_inner = { 0.2f, 0.6f, 1.0f, 1.0f };
		ellipse1.color.border_outer = { 0.8f, 0.9f, 1.0f, 1.0f };

		ellipse2.local = {
			{},
			{},
			{ 150.0f, 150.0f }
		};
		ellipse2.dimension.width = 1;
		ellipse2.dimension.height = 3;
		ellipse2.dimension.border = 0.4f;
		ellipse2.color.fill_inner = { 1.0f, 0.9f, 0.8f, 0.5f };
		ellipse2.color.fill_outer = { 1.0f, 0.6f, 0.2f, 1.0f };
		ellipse2.color.border_inner = { 0.0f, 0.0f, 0.0f, 1.0f };
		ellipse2.color.border_outer = { 0.8f, 0.9f, 1.0f, 0.0f };
	}

	EllipsePair::EllipsePair(Constructor c) :
		ellipse1(reg::load_ellipse(c.ellipse1)),
		ellipse2(reg::load_ellipse(c.ellipse2))
	{}

	void EllipsePair::draw(bool flush_ellipses) const
	{
		ellipse1.draw();
		ellipse2.draw();
		if (flush_ellipses)
			context::render_ellipses();
	}
}
