#pragma once

#include "Olympian.h"

namespace oly::gen
{
	struct BKG
	{
		rendering::Polygon bkg_rect;

		BKG();

		void draw(bool flush_polygons) const;
	};
}
