#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Sprites.h"

namespace oly::gen
{
	struct SpriteMatch
	{
		static void free_constructor();

		Transformer2D transformer;
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		rendering::Sprite sprite0;
		rendering::Sprite sprite2;

		SpriteMatch();
		SpriteMatch(const SpriteMatch&) = default;
		SpriteMatch(SpriteMatch&&) = default;
		SpriteMatch& operator=(const SpriteMatch&) = default;
		SpriteMatch& operator=(SpriteMatch&&) = default;

		void draw(bool flush_sprites) const;

		void on_tick() const;
	};
}
