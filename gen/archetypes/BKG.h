#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Polygons.h"

namespace oly::gen
{
	struct BKG
	{
		Transformer2D transformer;
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		rendering::Polygon bkg_rect;

	private:
		struct Constructor
		{
			struct
			{
				Transform2D local;
				std::unique_ptr<TransformModifier2D> modifier;
			} transformer;
			reg::params::Polygon bkg_rect;

			Constructor();
		};

	public:
		BKG(Constructor = {});

		void draw(bool flush_polygons) const;

		void on_tick() const;
	};
}
