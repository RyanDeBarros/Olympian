#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Polygons.h"

namespace oly::gen
{
	struct PolygonCrop
	{
		rendering::Polygon pentagon1;
		rendering::Polygon pentagon2;
		rendering::PolyComposite bordered_quad;

    private:
        struct Constructor
        {
			reg::params::Polygon pentagon1;
			reg::params::Polygon pentagon2;
			reg::params::PolyComposite bordered_quad;

			Constructor();
		};

	public:
		PolygonCrop(Constructor = {});

		void draw(bool flush_polygons) const;

		void on_tick() const;
	};
}
