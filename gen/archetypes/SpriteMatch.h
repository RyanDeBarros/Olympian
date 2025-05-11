#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Sprites.h"

namespace oly::gen
{
	struct SpriteMatch
	{
		rendering::Sprite sprite0;
		rendering::Sprite sprite2;

	private:
		struct Constructor
		{
			reg::params::Sprite sprite0;
			reg::params::Sprite sprite2;

			Constructor();
		};

	public:
		SpriteMatch(Constructor = {});

		void draw(bool flush_sprites) const;

		void on_tick() const;
	};
}
