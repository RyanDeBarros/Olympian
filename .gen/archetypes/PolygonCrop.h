#pragma once

#include "Olympian.h"

namespace oly::gen
{
    struct PolygonCrop
    {
        Transformer2D transformer;
		rendering::PolygonRef pentagon1;
		rendering::PolygonRef pentagon2;
		rendering::PolyCompositeRef bordered_quad;

        PolygonCrop();
        PolygonCrop(const PolygonCrop&) = default;
        PolygonCrop(PolygonCrop&&) = default;
        PolygonCrop& operator=(const PolygonCrop&) = default;
        PolygonCrop& operator=(PolygonCrop&&) = default;

        const Transform2D& get_local() const { return transformer.get_local(); }
        Transform2D& set_local() { return transformer.set_local(); }

        void draw(bool flush_polygons) const;

        void on_tick() const;
    };
}
