#pragma once

#include "Olympian.h"

namespace oly::gen
{
    struct Jumble
    {
        Transformer2D transformer;
		rendering::SpriteRef sprite3;
		rendering::SpriteRef sprite4;
		rendering::SpriteRef sprite5;
		rendering::SpriteRef sprite1;
		rendering::SpriteRef godot_icon;
		rendering::SpriteRef knight;
		rendering::PolyCompositeRef concave_shape;
		rendering::NGonRef octagon;
		rendering::ParagraphRef test_text;
		rendering::SpriteAtlasRef atlased_knight;
		rendering::TileMapRef grass_tilemap;
		rendering::SpriteNonantRef nonant_panel;

        Jumble();
        Jumble(const Jumble&) = default;
        Jumble(Jumble&&) = default;
        Jumble& operator=(const Jumble&) = default;
        Jumble& operator=(Jumble&&) = default;

        const Transform2D& get_local() const { return transformer.get_local(); }
        Transform2D& set_local() { return transformer.set_local(); }

        void draw(bool flush_sprites) const;

        void on_tick() const;
    };
}
