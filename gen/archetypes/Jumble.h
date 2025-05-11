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
		rendering::SpriteAtlasExtension atlased_knight;
		rendering::TileMap grass_tilemap;

	private:
		struct Constructor
		{
			struct
			{
				Transform2D local;
				std::unique_ptr<TransformModifier2D> modifier;
			} transformer;
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
