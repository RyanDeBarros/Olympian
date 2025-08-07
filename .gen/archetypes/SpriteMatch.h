#pragma once

#include "Olympian.h"

namespace oly::gen
{
    struct SpriteMatch
    {
        Transformer2D transformer;
		rendering::SpriteRef sprite0;
		rendering::SpriteRef sprite2;

        SpriteMatch();
        SpriteMatch(const SpriteMatch&) = default;
        SpriteMatch(SpriteMatch&&) = default;
        SpriteMatch& operator=(const SpriteMatch&) = default;
        SpriteMatch& operator=(SpriteMatch&&) = default;

        const Transform2D& get_local() const { return transformer.get_local(); }
        Transform2D& set_local() { return transformer.set_local(); }

        void draw(bool flush_sprites) const;

        void on_tick() const;
    };
}
