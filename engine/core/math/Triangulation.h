#pragma once

#include "core/math/Geometry.h"

namespace oly::math
{
	typedef std::vector<glm::uvec3> Triangulation;

	struct Edge
	{
		glm::uint a, b;

		Edge(glm::uint a, glm::uint b);

		bool operator==(const Edge&) const = default;
	};

	struct EdgeHash
	{
		size_t operator()(const Edge& e) const
		{
			return std::hash<glm::uint>{}(e.a) ^ std::hash<glm::uint>{}(e.b);
		}
	};

	extern std::unordered_map<Edge, std::vector<glm::uint>, EdgeHash> build_adjecency(const Triangulation& triangulation);
	extern Triangulation triangulate(const Polygon2D& polygon, bool increasing = true, int starting_offset = 0, int ear_cycle = 0);
	extern glm::uint get_first_ear(const Polygon2D& polygon, int starting_offset = 0);
	extern std::vector<Triangulation> convex_decompose_triangulation(const Polygon2D& polygon);
	extern std::vector<Triangulation> convex_decompose_triangulation(const Polygon2D& polygon, const Triangulation& triangulation);
	extern std::vector<std::pair<Polygon2D, Triangulation>> convex_decompose_polygon(const Polygon2D& polygon);
	extern std::vector<std::pair<Polygon2D, Triangulation>> convex_decompose_polygon(const Polygon2D& polygon, const Triangulation& triangulation);
	extern std::vector<std::pair<Polygon2D, Triangulation>> decompose_polygon(const Polygon2D& polygon, const std::vector<Triangulation>& triangulations);
	extern std::vector<Polygon2D> convex_decompose_polygon_without_triangulation(const Polygon2D& polygon);
	extern std::vector<Polygon2D> convex_decompose_polygon_without_triangulation(const Polygon2D& polygon, const Triangulation& triangulation);
	extern std::vector<Polygon2D> decompose_polygon_without_triangulation(const Polygon2D& polygon, const std::vector<Triangulation>& triangulations);
}
