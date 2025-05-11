#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Polygons.h"

namespace oly::gen
{
	struct BKG
	{
		static void free_constructor();

		Transformer2D transformer;
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		rendering::Polygon bkg_rect;

		BKG();
		BKG(const BKG&) = default;
		BKG(BKG&&) = default;
		BKG& operator=(const BKG&) = default;
		BKG& operator=(BKG&&) = default;

		void draw(bool flush_polygons) const;

		void on_tick() const;
	};
}
