#pragma once

#include "core/math/Geometry.h"

namespace oly::math
{
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

	typedef std::vector<glm::uvec3> Triangulation;
	extern std::unordered_map<Edge, std::vector<glm::uint>, EdgeHash> build_adjecency(const Triangulation& triangulation);
	extern Triangulation triangulate(const Polygon2D& polygon, bool increasing = true, int starting_offset = 0, int ear_cycle = 0);

	template<bool Triangulation, bool Polygon>
	struct Decompose
	{
		static_assert(Triangulation || Polygon, "Decompose must decompose at least one of Triangulation and Polygon.");
	};

	template<>
	struct Decompose<true, false>
	{
		std::vector<Triangulation> operator()(const Polygon2D& polygon) const;
		std::vector<Triangulation> operator()(const Polygon2D& polygon, const Triangulation& triangulation) const;
	};

	using DecomposeTriangulation = Decompose<true, false>;

	template<>
	struct Decompose<true, true>
	{
		std::vector<std::pair<Polygon2D, Triangulation>> operator()(const Polygon2D& polygon) const;
		std::vector<std::pair<Polygon2D, Triangulation>> operator()(const Polygon2D& polygon, const Triangulation& triangulation) const;
		std::vector<std::pair<Polygon2D, Triangulation>> operator()(const Polygon2D& polygon, const std::vector<Triangulation>& triangulations) const;
	};

	using DecomposeAll = Decompose<true, true>;

	template<>
	struct Decompose<false, true>
	{
		std::vector<Polygon2D> operator()(const Polygon2D& polygon) const;
		std::vector<Polygon2D> operator()(const Polygon2D& polygon, const Triangulation& triangulation) const;
		std::vector<Polygon2D> operator()(const Polygon2D& polygon, const std::vector<Triangulation>& triangulations) const;
	};

	using DecomposePolygon = Decompose<false, true>;
}
