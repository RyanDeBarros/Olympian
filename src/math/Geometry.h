#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace oly
{
	namespace math
	{
		struct Barycentric : glm::vec3
		{
			float root() const { return x; }
			float& root() { return x; }
			float prev() const { return y; }
			float& prev() { return y; }
			float next() const { return z; }
			float& next() { return z; }

			bool inside() const { return x >= 0.0f && y >= 0.0f && z >= 0.0f; }
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
			Barycentric barycentric(glm::vec2 point) const; // (root, prev, next)
		};

		extern float cross(glm::vec2 u, glm::vec2 v);
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

		struct Triangulation
		{
			glm::uint index_offset = 0;
			std::vector<glm::uvec3> faces;

			void set_index_offset(glm::uint index_offset);

			size_t num_indices() const { return 3 * faces.size(); }
		};

		struct TriangulatedPolygon2D
		{
			Polygon2D polygon;
			Triangulation triangulation;
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
		extern TriangulatedPolygon2D create_triangle(glm::vec4 color,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_triangle_border(glm::vec4 color, float border, BorderPivot border_pivot,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_quad(glm::vec4 color,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_quad_border(glm::vec4 color, float border, BorderPivot border_pivot,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_pentagon(glm::vec4 color,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_pentagon_border(glm::vec4 color, float border, BorderPivot border_pivot,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_hexagon(glm::vec4 color,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_hexagon_border(glm::vec4 color, float border, BorderPivot border_pivot,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_ngon(glm::vec4 color, const std::vector<glm::vec2>& points, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_ngon(glm::vec4 color, std::vector<glm::vec2>&& points, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_ngon(const std::vector<glm::vec4>& colors, const std::vector<glm::vec2>& points, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_ngon(std::vector<glm::vec4>&& colors, std::vector<glm::vec2>& points, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_ngon_border(glm::vec4 color, float border, BorderPivot border_pivot, const std::vector<glm::vec2>& points, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_ngon_border(const std::vector<glm::vec4>& colors, float border, BorderPivot border_pivot, const std::vector<glm::vec2>& points, glm::uint index_offset = 0);
		extern TriangulatedPolygon2D create_ngon_border(std::vector<glm::vec4>&& colors, float border, BorderPivot border_pivot, const std::vector<glm::vec2>& points, glm::uint index_offset = 0);

		typedef std::vector<TriangulatedPolygon2D> Polygon2DComposite;
		extern Polygon2DComposite split_polygon_composite(const TriangulatedPolygon2D& tp, glm::uint max_degree);
		extern void split_polygon_composite(Polygon2DComposite& composite, glm::uint max_degree);

		extern Polygon2DComposite create_bordered_triangle (glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::uint max_degree, glm::uint index_offset = 0);
		extern Polygon2DComposite create_bordered_quad(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::uint max_degree, glm::uint index_offset = 0);
		extern Polygon2DComposite create_bordered_pentagon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::uint max_degree, glm::uint index_offset = 0);
		extern Polygon2DComposite create_bordered_hexagon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
			glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6, glm::uint max_degree, glm::uint index_offset = 0);
		extern Polygon2DComposite create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
			const std::vector<glm::vec2>& points, glm::uint max_degree, glm::uint index_offset = 0);
		extern Polygon2DComposite create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
			std::vector<glm::vec2>&& points, glm::uint max_degree, glm::uint index_offset = 0);
		extern Polygon2DComposite create_bordered_ngon(const std::vector<glm::vec4>& fill_colors, const std::vector<glm::vec4>& border_colors, float border, BorderPivot border_pivot,
			const std::vector<glm::vec2>& points, glm::uint max_degree, glm::uint index_offset = 0);
		extern Polygon2DComposite create_bordered_ngon(std::vector<glm::vec4>&& fill_colors, std::vector<glm::vec4>&& border_colors, float border, BorderPivot border_pivot,
			std::vector<glm::vec2>&& points, glm::uint max_degree, glm::uint index_offset = 0);

		extern Triangulation ear_clipping(glm::uint index_offset, const std::vector<glm::vec2>& polygon, bool increasing = true, int starting_offset = 0, int ear_cycle = 0);
		extern size_t get_first_ear(const std::vector<glm::vec2>& polygon, int starting_offset = 0);
		// TODO algorithm to compute closest mutually visible vertex to a given vertex in a polygon. see ear clipping triangulation paper
	}
}
