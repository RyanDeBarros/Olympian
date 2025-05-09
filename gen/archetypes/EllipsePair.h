#pragma once

#include "Olympian.h"

namespace oly::gen
{
	struct EllipsePair
	{
		rendering::Ellipse ellipse1, ellipse2;

		EllipsePair();

		void draw(bool flush_ellipses) const;
	};
}
