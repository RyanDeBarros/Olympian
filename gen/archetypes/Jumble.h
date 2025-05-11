#pragma once

#include "Olympian.h"

#include "registries/graphics/primitives/Sprites.h"
#include "registries/graphics/primitives/Polygons.h"
#include "registries/graphics/text/Paragraphs.h"
#include "registries/graphics/extensions/SpriteAtlases.h"
#include "registries/graphics/extensions/TileMaps.h"

namespace oly::gen
{
	struct Jumble
	{
		rendering::Sprite sprite3;
		rendering::Sprite sprite4;
		rendering::Sprite sprite5;
		rendering::Sprite sprite1;
		rendering::Sprite godot_icon;
		rendering::Sprite knight;
		rendering::PolyComposite concave_shape;
		rendering::NGon octagon;
		rendering::Paragraph test_text;
		rendering::SpriteAtlasExtension atlased_knight;
		rendering::TileMap grass_tilemap;

	private:
		struct Constructor
		{
			reg::params::Sprite sprite3;
			reg::params::Sprite sprite4;
			reg::params::Sprite sprite5;
			reg::params::Sprite sprite1;
			reg::params::Sprite godot_icon;
			reg::params::Sprite knight;
			reg::params::PolyComposite concave_shape;
			reg::params::NGon octagon;
			reg::params::Paragraph test_text;
			reg::params::SpriteAtlas atlased_knight;
			reg::params::TileMap grass_tilemap;

			Constructor();
		};

	public:
		Jumble(Constructor = {});

		void draw(bool flush_text) const;

		void on_tick() const;
	};
}
