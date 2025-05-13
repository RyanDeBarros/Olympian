#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Sprites.h"
#include "registries/graphics/primitives/Polygons.h"
#include "registries/graphics/text/Paragraphs.h"
#include "registries/graphics/extensions/SpriteAtlases.h"
#include "registries/graphics/extensions/TileMaps.h"
#include "registries/graphics/extensions/SpriteNonants.h"

namespace oly::gen
{
	struct Jumble
	{
		static void free_constructor();

		Transformer2D transformer;
		const Transform2D& get_local() const { return transformer.get_local(); }
		Transform2D& set_local() { return transformer.set_local(); }

		rendering::Sprite sprite3;
		rendering::Sprite sprite4;
		rendering::Sprite sprite5;
		rendering::Sprite sprite1;
		rendering::Sprite godot_icon;
		rendering::Sprite knight;
		rendering::PolyComposite concave_shape;
		rendering::NGon octagon;
		rendering::Paragraph test_text;
		rendering::SpriteAtlas atlased_knight;
		rendering::TileMap grass_tilemap;
		rendering::SpriteNonant nonant_panel;

		Jumble();
		Jumble(const Jumble&) = default;
		Jumble(Jumble&&) = default;
		Jumble& operator=(const Jumble&) = default;
		Jumble& operator=(Jumble&&) = default;

		void draw(bool flush_sprites) const;

		void on_tick() const;
	};
}
