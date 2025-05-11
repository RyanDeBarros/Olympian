#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Polygons.h"

namespace oly::gen
{
	struct PolygonCrop
	{
		Transformer2D transformer;
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		rendering::Polygon pentagon1;
		rendering::Polygon pentagon2;
		rendering::PolyComposite bordered_quad;

	private:
		struct Constructor
		{
			struct
			{
				Transform2D local;
				std::unique_ptr<TransformModifier2D> modifier;
			} transformer;
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
