#pragma once

#include <unordered_map>

#include "external/GLM.h"

namespace oly::cmath
{
	struct Polygon2D
	{
		std::vector<glm::vec2> points;
		std::vector<glm::vec4> colors;

		void fill_colors();
		bool valid() const;
		void merge(const Polygon2D& other);
	};

	extern size_t num_triangulated_indices(const Polygon2D& polygon);

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

	struct TriangulatedPolygon2D
	{
		Polygon2D polygon;
		Triangulation triangulation;

		TriangulatedPolygon2D() = default;
		TriangulatedPolygon2D(const Polygon2D& polygon, const Triangulation& triangulation) : polygon(polygon), triangulation(triangulation) {}
		TriangulatedPolygon2D(Polygon2D&& polygon, Triangulation&& triangulation) : polygon(std::move(polygon)), triangulation(std::move(triangulation)) {}
		TriangulatedPolygon2D(const Polygon2D& polygon);
		TriangulatedPolygon2D(Polygon2D&& polygon);
	};

	struct BorderPivot
	{
		float v = 0.5f;

		constexpr BorderPivot(float v = 0.5f) : v(v) {}

		static const BorderPivot OUTER;
		static const BorderPivot MIDDLE;
		static const BorderPivot INNER;
	};
	inline const BorderPivot BorderPivot::OUTER = { 0.0f };
	inline const BorderPivot BorderPivot::MIDDLE = { 0.5f };
	inline const BorderPivot BorderPivot::INNER = { 1.0f };

	struct BorderPointPair
	{
		glm::vec2 inner;
		glm::vec2 outer;
	};
	extern BorderPointPair border_points(glm::vec2 point, float border, BorderPivot border_pivot, glm::vec2 prev_point, glm::vec2 next_point);
	extern void triangulate_border(const std::vector<glm::vec2>& border, Triangulation& triangulation);
	extern TriangulatedPolygon2D create_triangle(glm::vec4 color,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
	extern TriangulatedPolygon2D create_triangle_border(glm::vec4 color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
	extern TriangulatedPolygon2D create_quad(glm::vec4 color,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4);
	extern TriangulatedPolygon2D create_quad_border(glm::vec4 color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4);
	extern TriangulatedPolygon2D create_pentagon(glm::vec4 color,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5);
	extern TriangulatedPolygon2D create_pentagon_border(glm::vec4 color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5);
	extern TriangulatedPolygon2D create_hexagon(glm::vec4 color,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6);
	extern TriangulatedPolygon2D create_hexagon_border(glm::vec4 color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6);
	extern TriangulatedPolygon2D create_ngon(glm::vec4 color, const std::vector<glm::vec2>& points);
	extern TriangulatedPolygon2D create_ngon(glm::vec4 color, std::vector<glm::vec2>&& points);
	extern TriangulatedPolygon2D create_ngon(const std::vector<glm::vec4>& colors, const std::vector<glm::vec2>& points);
	extern TriangulatedPolygon2D create_ngon(std::vector<glm::vec4>&& colors, std::vector<glm::vec2>& points);
	extern TriangulatedPolygon2D create_ngon_border(glm::vec4 color, float border, BorderPivot border_pivot, const std::vector<glm::vec2>& points);
	extern TriangulatedPolygon2D create_ngon_border(const std::vector<glm::vec4>& colors, float border, BorderPivot border_pivot, const std::vector<glm::vec2>& points);
	extern TriangulatedPolygon2D create_ngon_border(std::vector<glm::vec4>&& colors, float border, BorderPivot border_pivot, const std::vector<glm::vec2>& points);

	typedef std::vector<TriangulatedPolygon2D> Polygon2DComposite;
	
	extern Polygon2DComposite create_bordered_triangle(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
	extern Polygon2DComposite create_bordered_quad(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4);
	extern Polygon2DComposite create_bordered_pentagon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5);
	extern Polygon2DComposite create_bordered_hexagon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6);
	extern Polygon2DComposite create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
		const std::vector<glm::vec2>& points);
	extern Polygon2DComposite create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
		std::vector<glm::vec2>&& points);
	extern Polygon2DComposite create_bordered_ngon(const std::vector<glm::vec4>& fill_colors, const std::vector<glm::vec4>& border_colors, float border, BorderPivot border_pivot,
		const std::vector<glm::vec2>& points);
	extern Polygon2DComposite create_bordered_ngon(std::vector<glm::vec4>&& fill_colors, std::vector<glm::vec4>&& border_colors, float border, BorderPivot border_pivot,
		std::vector<glm::vec2>&& points);

	struct NGonBase
	{
		std::vector<glm::vec2> points;
		std::vector<glm::vec4> fill_colors;
		std::vector<glm::vec4> border_colors;
		float border_width = 0.0f;
		BorderPivot border_pivot = BorderPivot::MIDDLE;

		Polygon2DComposite composite() const;
		Polygon2DComposite bordered_composite() const;
	};
}
