#pragma once

#include "Olympian.h"

namespace oly::gen
{
	struct Jumble
	{
		rendering::Sprite sprite3, sprite4, sprite5;
		rendering::NGon octagon;
		rendering::Sprite sprite1, godot_icon_3_0, knight;
		rendering::PolyComposite concave_shape;
		rendering::SpriteAtlasExtension atlased_knight;
		rendering::TileMap grass_tilemap;
		rendering::Paragraph test_text;

		Jumble();

		void draw(bool flush_text) const;

		void on_tick() const;
	};
}
