#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Ellipses.h"

namespace oly::gen
{
	struct EllipsePair
	{
		Transformer2D transformer;
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		rendering::Ellipse ellipse1;
		rendering::Ellipse ellipse2;

	private:
		struct Constructor
		{
			struct
			{
				Transform2D local;
				std::unique_ptr<TransformModifier2D> modifier;
			} transformer;
			reg::params::Ellipse ellipse1;
			reg::params::Ellipse ellipse2;

			Constructor();
		};

	public:
		EllipsePair(Constructor = {});

		void draw(bool flush_ellipses) const;

		void on_tick() const;
	};
}
