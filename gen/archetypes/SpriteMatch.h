#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Sprites.h"

namespace oly::gen
{
	struct SpriteMatch
	{
		Transformer2D transformer;
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		rendering::Sprite sprite0;
		rendering::Sprite sprite2;

	private:
		struct Constructor
		{
			struct
			{
				Transform2D local;
				std::unique_ptr<TransformModifier2D> modifier;
			} transformer;
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
