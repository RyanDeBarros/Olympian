#include "EllipsePair.h"

namespace oly::gen
{
	EllipsePair::EllipsePair() :
		ellipse1(reg::load_ellipse(context::load_toml("assets/ellipses/ellipse1.toml")["ellipse"])),
		ellipse2(reg::load_ellipse(context::load_toml("assets/ellipses/ellipse2.toml")["ellipse"]))
	{}

	void EllipsePair::draw(bool flush_ellipses) const
	{
		ellipse1.draw();
		ellipse2.draw();
		if (flush_ellipses)
			context::render_ellipses();
	}
}
