#pragma once

#include "physics/collision/objects/Compound.h"
#include "physics/collision/objects/BVH.h"
#include "core/math/Triangulation.h"

namespace oly::col2d
{
	struct PolygonCollision
	{
		math::Polygon2D concave_polygon;

		Primitive as_convex_primitive() const
		{
			return Primitive(ConvexHull::wrap(concave_polygon));
		}

		TPrimitive as_convex_tprimitive() const
		{
			return TPrimitive(ConvexHull::wrap(concave_polygon));
		}

		Compound as_convex_compound() const
		{
			Compound compound;
			std::vector<math::Polygon2D> convex_polygons = math::Decompose<false, true>{}(concave_polygon);
			compound.elements.resize(convex_polygons.size());
			for (size_t i = 0; i < convex_polygons.size(); ++i)
			{
				math::simplify(convex_polygons[i], LINEAR_TOLERANCE);
				compound.elements[i] = ConvexHull(std::move(convex_polygons[i]));
			}
			return compound;
		}

		TCompound as_convex_tcompound() const
		{
			TCompound compound;
			std::vector<math::Polygon2D> convex_polygons = math::Decompose<false, true>{}(concave_polygon);
			compound.set_compound().elements.resize(convex_polygons.size());
			for (size_t i = 0; i < convex_polygons.size(); ++i)
			{
				math::simplify(convex_polygons[i], LINEAR_TOLERANCE);
				compound.set_compound().elements[i] = ConvexHull(std::move(convex_polygons[i]));
			}
			return compound;
		}

		template<typename Shape>
		BVH<Shape> as_convex_bvh() const
		{
			BVH<Shape> bvh;
			std::vector<math::Polygon2D> convex_polygons = math::Decompose<false, true>{}(concave_polygon);
			bvh.elements.resize(convex_polygons.size());
			for (size_t i = 0; i < convex_polygons.size(); ++i)
			{
				math::simplify(convex_polygons[i], LINEAR_TOLERANCE);
				bvh.set_elements()[i] = ConvexHull(std::move(convex_polygons[i]));
			}
			return bvh;
		}

		template<typename Shape>
		TBVH<Shape> as_convex_tbvh() const
		{
			TBVH<Shape> bvh;
			std::vector<math::Polygon2D> convex_polygons = math::Decompose<false, true>{}(concave_polygon);
			bvh.elements.resize(convex_polygons.size());
			for (size_t i = 0; i < convex_polygons.size(); ++i)
			{
				math::simplify(convex_polygons[i], LINEAR_TOLERANCE);
				bvh.set_elements()[i] = ConvexHull(std::move(convex_polygons[i]));
			}
			return bvh;
		}
	};
}
