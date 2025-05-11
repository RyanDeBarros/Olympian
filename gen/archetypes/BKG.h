#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Polygons.h"

namespace oly::gen
{
	struct BKG
	{
		rendering::Polygon bkg_rect;

	private:
		struct Constructor
		{
			reg::params::Polygon bkg_rect;

			Constructor();
		};

	public:
		BKG(Constructor = {});

		void draw(bool flush_polygons) const;

		void on_tick() const;
	};
}
