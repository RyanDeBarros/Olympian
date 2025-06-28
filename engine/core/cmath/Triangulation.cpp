#include "Triangulation.h"

#include "core/base/Assert.h"
#include "core/math/Triangulation.h"

namespace oly::cmath
{
	Polygon2DComposite Decompose::operator()(const Polygon2D& polygon) const
	{
		OLY_ASSERT(polygon.points.size() >= 3);
		std::vector<math::Triangulation> decomposition = math::Decompose<true, false>{}(polygon.points);
		return Decompose{}(polygon, decomposition);
	}

	Polygon2DComposite Decompose::operator()(const Polygon2D& polygon, const math::Triangulation& triangulation) const
	{
		OLY_ASSERT(polygon.points.size() >= 3);
		std::vector<math::Triangulation> decomposition = math::Decompose<true, false>{}(polygon.points, triangulation);
		return Decompose{}(polygon, decomposition);
	}

	Polygon2DComposite Decompose::operator()(const Polygon2D& polygon, const std::vector<math::Triangulation>& triangulations) const
	{
		OLY_ASSERT(polygon.points.size() >= 3);
		Polygon2DComposite composite;
		composite.reserve(triangulations.size());

		for (const math::Triangulation& triangulation : triangulations)
		{
			TriangulatedPolygon2D subpolygon;
			std::unordered_map<glm::uint, glm::uint> point_indices;
			for (glm::uvec3 face : triangulation)
			{
				glm::uvec3 new_face{};
				for (glm::length_t i = 0; i < 3; ++i)
				{
					if (!point_indices.count(face[i]))
					{
						point_indices[face[i]] = (glm::uint)point_indices.size();
						subpolygon.polygon.points.push_back(polygon.points[face[i]]);
						subpolygon.polygon.colors.push_back(i < polygon.colors.size() ? polygon.colors[face[i]] : polygon.colors[0]);
					}
					new_face[i] = point_indices[face[i]];
				}
				subpolygon.triangulation.push_back(new_face);
			}
			composite.push_back(std::move(subpolygon));
		}

		return composite;
	}

	Polygon2DComposite Decompose::operator()(const math::Polygon2D& polygon) const
	{
		auto decomposition = math::Decompose<true, true>{}(polygon);
		Polygon2DComposite composite;
		composite.reserve(decomposition.size());
		for (auto& subconvex : decomposition)
		{
			TriangulatedPolygon2D tp;
			tp.polygon.points = std::move(subconvex.first);
			tp.polygon.colors = { glm::vec4(1.0f) };
			tp.triangulation = std::move(subconvex.second);
			composite.push_back(std::move(tp));
		}
		return composite;
	}
}
