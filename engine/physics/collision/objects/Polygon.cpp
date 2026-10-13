#include "Polygon.h"

namespace oly::col2d
{
    Primitive PolygonCollision::as_convex_primitive() const
    {
        return Primitive(ConvexHull::wrap(concave_polygon));
    }

    TPrimitive PolygonCollision::as_convex_tprimitive() const
    {
        return TPrimitive(ConvexHull::wrap(concave_polygon));
    }

    Compound PolygonCollision::as_convex_compound() const
    {
        Compound compound;
        std::vector<math::Polygon2D> convex_polygons = math::Decompose<false, true>{}(concave_polygon);
        compound.elements.resize(convex_polygons.size());
        for (size_t i = 0; i < convex_polygons.size(); ++i)
        {
            math::simplify(convex_polygons[i], kLinearTolerance);
            compound.elements[i] = ConvexHull(std::move(convex_polygons[i]));
        }
        return compound;
    }

    TCompound PolygonCollision::as_convex_tcompound() const
    {
        TCompound compound;
        std::vector<math::Polygon2D> convex_polygons = math::Decompose<false, true>{}(concave_polygon);
        compound.set_compound().elements.resize(convex_polygons.size());
        for (size_t i = 0; i < convex_polygons.size(); ++i)
        {
            math::simplify(convex_polygons[i], kLinearTolerance);
            compound.set_compound().elements[i] = ConvexHull(std::move(convex_polygons[i]));
        }
        return compound;
    }
}
