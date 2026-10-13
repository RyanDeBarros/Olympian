#pragma once

#include "core/math/Triangulation.h"

#include "physics/collision/objects/BVH.h"
#include "physics/collision/objects/Compound.h"

namespace oly::col2d
{
	struct PolygonCollision
	{
		math::Polygon2D concave_polygon;

        Primitive as_convex_primitive() const;
        TPrimitive as_convex_tprimitive() const;
        Compound as_convex_compound() const;
        TCompound as_convex_tcompound() const;

		template<typename Shape>
		BVH<Shape> as_convex_bvh() const
		{
			BVH<Shape> bvh;
			std::vector<math::Polygon2D> convex_polygons = math::Decompose<false, true>{}(concave_polygon);
			bvh.elements.resize(convex_polygons.size());
			for (size_t i = 0; i < convex_polygons.size(); ++i)
			{
				math::simplify(convex_polygons[i], kLinearTolerance);
				bvh.set_elements()[i] = ConvexHull(std::move(convex_polygons[i]));
			}
			return bvh;
		}

		template<typename Shape>
		TBVH<Shape> as_convex_tbvh() const
		{
			TBVH<Shape> bvh;
			std::vector<math::Polygon2D> convex_polygons = math::Decompose<false, true>{}(concave_polygon);
			bvh.set_elements().resize(convex_polygons.size());
			for (size_t i = 0; i < convex_polygons.size(); ++i)
			{
				math::simplify(convex_polygons[i], kLinearTolerance);
				bvh.set_elements()[i] = ConvexHull(std::move(convex_polygons[i]));
			}
			return bvh;
		}
	};
}
