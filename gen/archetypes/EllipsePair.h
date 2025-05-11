#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Ellipses.h"

namespace oly::gen
{
	struct EllipsePair
	{
		static void free_constructor();

		Transformer2D transformer;
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		rendering::Ellipse ellipse1;
		rendering::Ellipse ellipse2;

		EllipsePair();
		EllipsePair(const EllipsePair&) = default;
		EllipsePair(EllipsePair&&) = default;
		EllipsePair& operator=(const EllipsePair&) = default;
		EllipsePair& operator=(EllipsePair&&) = default;

		void draw(bool flush_ellipses) const;

		void on_tick() const;
	};
}
