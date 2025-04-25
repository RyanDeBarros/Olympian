#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>
#include <algorithm>

namespace oly
{
	namespace math
	{
		namespace coordinates
		{
			inline glm::vec2 to_polar(glm::vec2 cartesian) { return { glm::length(cartesian), glm::atan(cartesian.y, cartesian.x) }; }
			inline glm::vec2 to_cartesian(glm::vec2 polar) { return polar.x * glm::vec2{ glm::cos(polar.y), glm::sin(polar.y) }; }
			inline glm::vec3 to_spherical(glm::vec3 cartesian) { float r = glm::length(cartesian);
					return r != 0.0f ? glm::vec3{ r, glm::atan(cartesian.y, cartesian.x), glm::acos(cartesian.z / r) } : glm::vec3{ 0.0f, 0.0f, 0.0f }; }
			inline glm::vec3 to_cartesian(glm::vec3 spherical) { float sphi = glm::sin(spherical.z); return spherical.x * glm::vec3{ glm::cos(spherical.y) * sphi, glm::sin(spherical.y) * sphi, glm::cosh(spherical.z)}; }
		}

		struct BBox2D
		{
			float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;

			glm::vec2 center() const { return 0.5f * glm::vec2{ x1 + x2, y1 + y2 }; }
			bool contains(glm::vec2 test) const { return test.x >= x1 && test.x <= x2 && test.y >= y1 && test.y <= y2; }
			glm::vec2 clamp(glm::vec2 pt) const { return { std::clamp(pt.x, x1, x2), std::clamp(pt.y, y1, y2) }; }
		};

		struct Barycentric : glm::vec3
		{
			using glm::vec3::vec;

			float root() const { return x; }
			float& root() { return x; }
			float prev() const { return y; }
			float& prev() { return y; }
			float next() const { return z; }
			float& next() { return z; }

			bool inside() const { return x >= 0.0f && y >= 0.0f && z >= 0.0f; }
			glm::vec2 point(glm::vec2 a, glm::vec2 b, glm::vec2 c) const { return x * a + y * b + z * c; }
		};

		struct Triangle2D
		{
			glm::vec2 root;
			glm::vec2 prev;
			glm::vec2 next;

			glm::vec2 dprev() const { return root - prev; }
			glm::vec2 dnext() const { return next - root; }
			float signed_area() const;
			float area() const { return std::abs(signed_area()); }
			Barycentric barycentric(glm::vec2 point) const;
			float cross() const;
		};

		struct DirectedLine2D
		{
			glm::vec2 anchor;
			glm::vec2 dir;

			bool intersect(const DirectedLine2D& other, glm::vec2 pt) const;
		};

		extern float cross(glm::vec2 u, glm::vec2 v);
		extern bool in_convex_sector(glm::vec2 u1, glm::vec2 u2, glm::vec2 test);
		extern float signed_area(const std::vector<glm::vec2>& points);
		
		template<typename T, enum glm::qualifier Q = glm::packed_highp>
		inline glm::vec<2, T, Q> reverse(glm::vec<2, T, Q> vec) { return { vec.y, vec.x }; }
		template<typename T, enum glm::qualifier Q = glm::packed_highp>
		inline glm::vec<3, T, Q> reverse(glm::vec<3, T, Q> vec) { return { vec.z, vec.y, vec.x }; }
		template<typename T, enum glm::qualifier Q = glm::packed_highp>
		inline glm::vec<4, T, Q> reverse(glm::vec<4, T, Q> vec) { return { vec.w, vec.z, vec.y, vec.x }; }
		
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
			size_t operator()(const oly::math::Edge& e) const
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
		extern Polygon2DComposite split_polygon_composite(const TriangulatedPolygon2D& tp, glm::uint max_degree);
		extern Polygon2DComposite split_polygon_composite(TriangulatedPolygon2D&& tp, glm::uint max_degree);
		extern void split_polygon_composite(Polygon2DComposite& composite, glm::uint max_degree);

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

			Polygon2DComposite composite(glm::uint max_degree) const;
			Polygon2DComposite bordered_composite(glm::uint max_degree) const;
		};

		extern Triangulation triangulate(const std::vector<glm::vec2>& polygon, bool increasing = true, int starting_offset = 0, int ear_cycle = 0);
		extern glm::uint get_first_ear(const std::vector<glm::vec2>& polygon, int starting_offset = 0);
		extern std::vector<Triangulation> convex_decompose_triangulation(const std::vector<glm::vec2>& polygon);
		extern std::vector<Triangulation> convex_decompose_triangulation(const std::vector<glm::vec2>& polygon, const Triangulation& triangulation);
		extern std::vector<std::pair<std::vector<glm::vec2>, Triangulation>> convex_decompose_polygon(const std::vector<glm::vec2>& polygon);
		extern std::vector<std::pair<std::vector<glm::vec2>, Triangulation>> convex_decompose_polygon(const std::vector<glm::vec2>& polygon, const Triangulation& triangulation);
		extern std::vector<std::pair<std::vector<glm::vec2>, Triangulation>> decompose_polygon(const std::vector<glm::vec2>& polygon, const std::vector<Triangulation>& triangulations);
		extern Polygon2DComposite composite_convex_decomposition(const std::vector<glm::vec2>& points);
	}
}
