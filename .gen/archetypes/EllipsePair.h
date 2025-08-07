#pragma once

#include "Olympian.h"

namespace oly::gen
{
    struct EllipsePair
    {
        Transformer2D transformer;
		rendering::EllipseRef ellipse1;
		rendering::EllipseRef ellipse2;

        EllipsePair();
        EllipsePair(const EllipsePair&) = default;
        EllipsePair(EllipsePair&&) = default;
        EllipsePair& operator=(const EllipsePair&) = default;
        EllipsePair& operator=(EllipsePair&&) = default;

        const Transform2D& get_local() const { return transformer.get_local(); }
        Transform2D& set_local() { return transformer.set_local(); }

        void draw(bool flush_ellipses) const;

        void on_tick() const;
    };
}
