#pragma once

#include "Olympian.h"

namespace oly::gen
{
    struct BKG
    {
        Transformer2D transformer;
		rendering::PolygonRef bkg_rect;

        BKG();
        BKG(const BKG&) = default;
        BKG(BKG&&) = default;
        BKG& operator=(const BKG&) = default;
        BKG& operator=(BKG&&) = default;

        const Transform2D& get_local() const { return transformer.get_local(); }
        Transform2D& set_local() { return transformer.set_local(); }

        void draw(bool flush_polygons) const;

        void on_tick() const;
    };
}
