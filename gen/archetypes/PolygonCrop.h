#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Polygons.h"

namespace oly::gen
{
	struct PolygonCrop
	{
		static void free_constructor();

		Transformer2D transformer;
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		rendering::Polygon pentagon1;
		rendering::Polygon pentagon2;
		rendering::PolyComposite bordered_quad;

		PolygonCrop();
		PolygonCrop(const PolygonCrop&) = default;
		PolygonCrop(PolygonCrop&&) = default;
		PolygonCrop& operator=(const PolygonCrop&) = default;
		PolygonCrop& operator=(PolygonCrop&&) = default;

		void draw(bool flush_polygons) const;

		void on_tick() const;
	};
}
