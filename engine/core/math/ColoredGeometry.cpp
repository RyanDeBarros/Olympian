#include "ColoredGeometry.h"

#include <set>

#include "core/base/Assert.h"
#include "core/math/Geometry.h"
#include "core/math/Triangulation.h"

namespace oly::cmath
{
	void Polygon2D::fill_colors()
	{
		if (colors.size() == 1)
			for (size_t i = 1; i < points.size(); ++i)
				colors.push_back(colors[0]);
	}

	bool Polygon2D::valid() const
	{
		return points.size() >= 3 && (colors.size() == 1 || colors.size() == points.size());
	}

	void Polygon2D::merge(const Polygon2D& other)
	{
		fill_colors();
		points.reserve(points.size() + other.points.size());
		colors.reserve(points.size());
		for (size_t i = 0; i < other.points.size(); ++i)
		{
			points.push_back(other.points[i]);
			colors.push_back(other.colors[i]);
		}
	}

	size_t num_triangulated_indices(const Polygon2D& polygon)
	{
		return polygon.points.size() >= 3 ? (polygon.points.size() - 2) * 3 : 0;
	}

	std::unordered_map<Edge, std::vector<glm::uint>, EdgeHash> build_adjecency(const Triangulation& triangulation)
	{
		std::unordered_map<Edge, std::vector<glm::uint>, EdgeHash> adjacency;
		for (glm::uint i = 0; i < triangulation.size(); ++i)
		{
			const auto& face = triangulation[i];
			adjacency[Edge(face[0], face[1])].push_back(i);
			adjacency[Edge(face[1], face[2])].push_back(i);
			adjacency[Edge(face[2], face[0])].push_back(i);
		}
		return adjacency;
	}

	Edge::Edge(glm::uint a, glm::uint b)
		: a(std::min(a, b)), b(std::max(a, b))
	{
	}

	TriangulatedPolygon2D::TriangulatedPolygon2D(const Polygon2D& polygon)
		: polygon(polygon)
	{
		triangulation = triangulate(polygon.points);
	}

	TriangulatedPolygon2D::TriangulatedPolygon2D(Polygon2D&& polygon)
		: polygon(std::move(polygon))
	{
		triangulation = triangulate(this->polygon.points);
	}

	BorderPointPair border_points(glm::vec2 point, float border, BorderPivot border_pivot, glm::vec2 prev_point, glm::vec2 next_point)
	{
		glm::vec2 v_prev = glm::normalize(prev_point - point);
		glm::vec2 v_next = glm::normalize(next_point - point);
		bool reflex = math::cross(v_prev, v_next) >= 0.0f;
		glm::vec2 n_prev{ v_prev.y, -v_prev.x };
		glm::vec2 n_next{ -v_next.y, v_next.x };
		if (reflex)
		{
			n_prev *= -1;
			n_next *= -1;
		}
		float b = border * border_pivot.v;
		float t = b * glm::length(n_next - n_prev) / glm::length(v_prev - v_next);
		glm::vec2 inner = point + b * n_next + t * v_next;
		b = border * (border_pivot.v - 1.0f);
		t = b * glm::length(n_next - n_prev) / glm::length(v_prev - v_next);
		glm::vec2 outer = point + b * n_next + t * v_next;
		return reflex ? BorderPointPair{ outer, inner } : BorderPointPair{ inner, outer };
	}

	void triangulate_border(const std::vector<glm::vec2>& border, Triangulation& triangulation)
	{
		OLY_ASSERT(border.size() >= 4 && border.size() % 2 == 0);
		bool ccw = math::signed_area(border) >= 0.0f;
		glm::uvec3 face{};

		face[0] = (glm::uint)border.size() - 2;
		face[1] = (glm::uint)border.size() - 1;
		face[2] = 1;
		triangulation.push_back(ccw ? face : math::reverse(face));
		face[0] = (glm::uint)border.size() - 2;
		face[1] = 1;
		face[2] = 0;
		triangulation.push_back(ccw ? face : math::reverse(face));
		for (glm::uint i = 1; i < border.size() / 2; ++i)
		{
			face[0] = 2 * i - 2;
			face[1] = 2 * i - 1;
			face[2] = 2 * i + 1;
			triangulation.push_back(ccw ? face : math::reverse(face));
			face[0] = 2 * i - 2;
			face[1] = 2 * i + 1;
			face[2] = 2 * i;
			triangulation.push_back(ccw ? face : math::reverse(face));
		}
	}

	TriangulatedPolygon2D create_triangle(glm::vec4 color, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
	{
		TriangulatedPolygon2D p;
		p.polygon.colors.push_back(color);
		p.polygon.points.reserve(3);
		p.polygon.points.push_back(p1);
		p.polygon.points.push_back(p2);
		p.polygon.points.push_back(p3);
		p.triangulation = triangulate(p.polygon.points);
		return p;
	}

	TriangulatedPolygon2D create_triangle_border(glm::vec4 color, float border, BorderPivot border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
	{
		TriangulatedPolygon2D p;
		p.polygon.colors.push_back(color);
		auto ps1 = border_points(p1, border, border_pivot, p3, p2);
		auto ps2 = border_points(p2, border, border_pivot, p1, p3);
		auto ps3 = border_points(p3, border, border_pivot, p2, p1);
		p.polygon.points.reserve(2 * 3);
		p.polygon.points.push_back(ps1.inner);
		p.polygon.points.push_back(ps1.outer);
		p.polygon.points.push_back(ps2.inner);
		p.polygon.points.push_back(ps2.outer);
		p.polygon.points.push_back(ps3.inner);
		p.polygon.points.push_back(ps3.outer);
		triangulate_border(p.polygon.points, p.triangulation);
		return p;
	}

	TriangulatedPolygon2D create_quad(glm::vec4 color, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4)
	{
		TriangulatedPolygon2D p;
		p.polygon.colors.push_back(color);
		p.polygon.points.reserve(4);
		p.polygon.points.push_back(p1);
		p.polygon.points.push_back(p2);
		p.polygon.points.push_back(p3);
		p.polygon.points.push_back(p4);
		p.triangulation = triangulate(p.polygon.points);
		return p;
	}

	TriangulatedPolygon2D create_quad_border(glm::vec4 color, float border, BorderPivot border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4)
	{
		TriangulatedPolygon2D p;
		p.polygon.colors.push_back(color);
		auto ps1 = border_points(p1, border, border_pivot, p4, p2);
		auto ps2 = border_points(p2, border, border_pivot, p1, p3);
		auto ps3 = border_points(p3, border, border_pivot, p2, p4);
		auto ps4 = border_points(p4, border, border_pivot, p3, p1);
		p.polygon.points.reserve(2 * 4);
		p.polygon.points.push_back(ps1.inner);
		p.polygon.points.push_back(ps1.outer);
		p.polygon.points.push_back(ps2.inner);
		p.polygon.points.push_back(ps2.outer);
		p.polygon.points.push_back(ps3.inner);
		p.polygon.points.push_back(ps3.outer);
		p.polygon.points.push_back(ps4.inner);
		p.polygon.points.push_back(ps4.outer);
		triangulate_border(p.polygon.points, p.triangulation);
		return p;
	}

	TriangulatedPolygon2D create_pentagon(glm::vec4 color, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5)
	{
		TriangulatedPolygon2D p;
		p.polygon.colors.push_back(color);
		p.polygon.points.reserve(5);
		p.polygon.points.push_back(p1);
		p.polygon.points.push_back(p2);
		p.polygon.points.push_back(p3);
		p.polygon.points.push_back(p4);
		p.polygon.points.push_back(p5);
		p.triangulation = triangulate(p.polygon.points);
		return p;
	}

	TriangulatedPolygon2D create_pentagon_border(glm::vec4 color, float border, BorderPivot border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5)
	{
		TriangulatedPolygon2D p;
		p.polygon.colors.push_back(color);
		auto ps1 = border_points(p1, border, border_pivot, p5, p2);
		auto ps2 = border_points(p2, border, border_pivot, p1, p3);
		auto ps3 = border_points(p3, border, border_pivot, p2, p4);
		auto ps4 = border_points(p4, border, border_pivot, p3, p5);
		auto ps5 = border_points(p5, border, border_pivot, p4, p1);
		p.polygon.points.reserve(2 * 5);
		p.polygon.points.push_back(ps1.inner);
		p.polygon.points.push_back(ps1.outer);
		p.polygon.points.push_back(ps2.inner);
		p.polygon.points.push_back(ps2.outer);
		p.polygon.points.push_back(ps3.inner);
		p.polygon.points.push_back(ps3.outer);
		p.polygon.points.push_back(ps4.inner);
		p.polygon.points.push_back(ps4.outer);
		p.polygon.points.push_back(ps5.inner);
		p.polygon.points.push_back(ps5.outer);
		triangulate_border(p.polygon.points, p.triangulation);
		return p;
	}

	TriangulatedPolygon2D create_hexagon(glm::vec4 color, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6)
	{
		TriangulatedPolygon2D p;
		p.polygon.colors.push_back(color);
		p.polygon.points.reserve(6);
		p.polygon.points.push_back(p1);
		p.polygon.points.push_back(p2);
		p.polygon.points.push_back(p3);
		p.polygon.points.push_back(p4);
		p.polygon.points.push_back(p5);
		p.polygon.points.push_back(p6);
		p.triangulation = triangulate(p.polygon.points);
		return p;
	}

	TriangulatedPolygon2D create_hexagon_border(glm::vec4 color, float border, BorderPivot border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6)
	{
		TriangulatedPolygon2D p;
		p.polygon.colors.push_back(color);
		auto ps1 = border_points(p1, border, border_pivot, p6, p2);
		auto ps2 = border_points(p2, border, border_pivot, p1, p3);
		auto ps3 = border_points(p3, border, border_pivot, p2, p4);
		auto ps4 = border_points(p4, border, border_pivot, p3, p5);
		auto ps5 = border_points(p5, border, border_pivot, p4, p6);
		auto ps6 = border_points(p6, border, border_pivot, p5, p1);
		p.polygon.points.reserve(2 * 6);
		p.polygon.points.push_back(ps1.inner);
		p.polygon.points.push_back(ps1.outer);
		p.polygon.points.push_back(ps2.inner);
		p.polygon.points.push_back(ps2.outer);
		p.polygon.points.push_back(ps3.inner);
		p.polygon.points.push_back(ps3.outer);
		p.polygon.points.push_back(ps4.inner);
		p.polygon.points.push_back(ps4.outer);
		p.polygon.points.push_back(ps5.inner);
		p.polygon.points.push_back(ps5.outer);
		p.polygon.points.push_back(ps6.inner);
		p.polygon.points.push_back(ps6.outer);
		triangulate_border(p.polygon.points, p.triangulation);
		return p;
	}

	TriangulatedPolygon2D create_ngon(glm::vec4 color, const std::vector<glm::vec2>& points)
	{
		OLY_ASSERT(points.size() >= 3);
		TriangulatedPolygon2D p;
		p.polygon.colors.push_back(color);
		p.polygon.points = points;
		p.triangulation = triangulate(p.polygon.points);
		return p;
	}

	TriangulatedPolygon2D create_ngon(glm::vec4 color, std::vector<glm::vec2>&& points)
	{
		OLY_ASSERT(points.size() >= 3);
		TriangulatedPolygon2D p;
		p.polygon.colors.push_back(color);
		p.polygon.points = std::move(points);
		p.triangulation = triangulate(p.polygon.points);
		return p;
	}

	TriangulatedPolygon2D create_ngon(const std::vector<glm::vec4>& colors, const std::vector<glm::vec2>& points)
	{
		OLY_ASSERT(points.size() >= 3);
		TriangulatedPolygon2D p;
		p.polygon.colors = colors;
		p.polygon.points = points;
		p.triangulation = triangulate(p.polygon.points);
		return p;
	}

	TriangulatedPolygon2D create_ngon(std::vector<glm::vec4>&& colors, std::vector<glm::vec2>& points)
	{
		OLY_ASSERT(points.size() >= 3);
		TriangulatedPolygon2D p;
		p.polygon.colors = std::move(colors);
		p.polygon.points = std::move(points);
		p.triangulation = triangulate(p.polygon.points);
		return p;
	}

	TriangulatedPolygon2D create_ngon_border(glm::vec4 color, float border, BorderPivot border_pivot, const std::vector<glm::vec2>& points)
	{
		glm::uint num_points = (glm::uint)points.size();
		OLY_ASSERT(num_points >= 3);
		TriangulatedPolygon2D p;
		p.polygon.colors.push_back(color);

		BorderPointPair bpt{};
		p.polygon.points.reserve(2 * num_points);
		bpt = border_points(points[0], border, border_pivot, points.back(), points[1]);
		p.polygon.points.push_back(bpt.inner);
		p.polygon.points.push_back(bpt.outer);
		for (size_t i = 1; i < num_points - 1; ++i)
		{
			bpt = border_points(points[i], border, border_pivot, points[i - 1], points[i + 1]);
			p.polygon.points.push_back(bpt.inner);
			p.polygon.points.push_back(bpt.outer);
		}
		bpt = border_points(points.back(), border, border_pivot, points[num_points - 2], points[0]);
		p.polygon.points.push_back(bpt.inner);
		p.polygon.points.push_back(bpt.outer);

		triangulate_border(p.polygon.points, p.triangulation);
		return p;
	}

	TriangulatedPolygon2D create_ngon_border(const std::vector<glm::vec4>& colors, float border, BorderPivot border_pivot, const std::vector<glm::vec2>& points)
	{
		auto p = create_ngon_border(glm::vec4{}, border, border_pivot, points);
		if (colors.size() == 1)
			p.polygon.colors = { colors[0] };
		else
		{
			bool solid_border_color = (colors.size() == points.size());
			p.polygon.colors = colors;
			if (solid_border_color)
			{
				p.polygon.colors.reserve(2 * p.polygon.colors.size());
				p.polygon.colors.insert(p.polygon.colors.end(), colors.rbegin(), colors.rend());
			}
		}
		return p;
	}

	TriangulatedPolygon2D create_ngon_border(std::vector<glm::vec4>&& colors, float border, BorderPivot border_pivot, const std::vector<glm::vec2>& points)
	{
		auto p = create_ngon_border(glm::vec4{}, border, border_pivot, points);
		if (colors.size() == 1)
			p.polygon.colors = { colors[0] };
		else
		{
			bool solid_border_color = (colors.size() == points.size());
			p.polygon.colors = std::move(colors);
			if (solid_border_color)
			{
				p.polygon.colors.reserve(2 * p.polygon.colors.size());
				colors = p.polygon.colors;
				p.polygon.colors.insert(p.polygon.colors.end(), colors.rbegin(), colors.rend());
			}
		}
		return p;
	}

	Polygon2DComposite split_polygon_composite(const TriangulatedPolygon2D& tp, glm::uint max_degree)
	{
		OLY_ASSERT(max_degree >= 3);
		Polygon2DComposite composite;
		glm::uint divisions = (glm::uint)tp.polygon.points.size() / max_degree;
		if (divisions == 0)
			return { tp };
		composite.reserve(divisions + 1);
		std::vector<glm::uvec3> faces;
		faces.reserve(max_degree);
		std::set<glm::uint> packed_points;

		static const auto add_subpolygon = [](Polygon2DComposite& composite, std::set<glm::uint>& packed_points, std::vector<glm::uvec3>& faces, glm::uint max_degree, const TriangulatedPolygon2D& superpolygon) {
			TriangulatedPolygon2D polygon;

			polygon.polygon.points.reserve(max_degree);
			std::unordered_map<glm::uint, glm::uint> local_vertex_indices;
			glm::uint i = 0;
			for (auto iter = packed_points.begin(); iter != packed_points.end(); ++iter)
			{
				polygon.polygon.points.push_back(superpolygon.polygon.points[*iter]);
				local_vertex_indices[*iter] = i++;
			}
			if (superpolygon.polygon.colors.size() == superpolygon.polygon.points.size())
			{
				polygon.polygon.colors.reserve(max_degree);
				for (auto iter = packed_points.begin(); iter != packed_points.end(); ++iter)
					polygon.polygon.colors.push_back(superpolygon.polygon.colors[*iter]);
			}
			else
				polygon.polygon.colors.push_back(superpolygon.polygon.colors[0]);

			polygon.triangulation.reserve(faces.size());
			for (glm::uvec3 face : faces)
			{
				face[0] = local_vertex_indices[face[0]];
				face[1] = local_vertex_indices[face[1]];
				face[2] = local_vertex_indices[face[2]];
				polygon.triangulation.push_back(face);
			}

			composite.push_back(std::move(polygon));
			packed_points.clear();
			faces.clear();
			};

		for (glm::uvec3 face : tp.triangulation)
		{
			glm::uint distinct_points = 0;
			if (!packed_points.count(face[0]))
				++distinct_points;
			if (!packed_points.count(face[1]))
				++distinct_points;
			if (!packed_points.count(face[2]))
				++distinct_points;

			if (packed_points.size() + distinct_points > max_degree)
				add_subpolygon(composite, packed_points, faces, max_degree, tp);

			packed_points.insert(face[0]);
			packed_points.insert(face[1]);
			packed_points.insert(face[2]);
			faces.push_back(face);
		}
		add_subpolygon(composite, packed_points, faces, max_degree, tp);
		return composite;
	}

	Polygon2DComposite split_polygon_composite(TriangulatedPolygon2D&& tp, glm::uint max_degree)
	{
		if (tp.polygon.points.size() <= max_degree)
			return { std::move(tp) };
		else
			return split_polygon_composite(tp, max_degree);
	}

	void split_polygon_composite(Polygon2DComposite& composite, glm::uint max_degree)
	{
		// LATER this simply calls split_polygon_composite(triangulated polygon, max_degree) and joins them all, but perhaps also implement merging subpolygons for optimal packing of points.
		Polygon2DComposite split_composite;
		for (auto& polygon : composite)
		{
			auto split_polygons = split_polygon_composite(polygon, max_degree);
			split_composite.insert(split_composite.end(), std::make_move_iterator(split_polygons.begin()), std::make_move_iterator(split_polygons.end()));
		}
		composite = std::move(split_composite);
	}

	Polygon2DComposite create_bordered_triangle(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
	{
		Polygon2DComposite composite;
		composite.push_back(create_triangle(fill_color, p1, p2, p3));
		composite.push_back(create_triangle_border(border_color, border, border_pivot, p1, p2, p3));
		return composite;
	}

	Polygon2DComposite create_bordered_quad(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4)
	{
		Polygon2DComposite composite;
		composite.push_back(create_quad(fill_color, p1, p2, p3, p4));
		composite.push_back(create_quad_border(border_color, border, border_pivot, p1, p2, p3, p4));
		return composite;
	}

	Polygon2DComposite create_bordered_pentagon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5)
	{
		Polygon2DComposite composite;
		composite.push_back(create_pentagon(fill_color, p1, p2, p3, p4, p5));
		composite.push_back(create_pentagon_border(border_color, border, border_pivot, p1, p2, p3, p4, p5));
		return composite;
	}

	Polygon2DComposite create_bordered_hexagon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot,
		glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6)
	{
		Polygon2DComposite composite;
		composite.push_back(create_hexagon(fill_color, p1, p2, p3, p4, p5, p6));
		composite.push_back(create_hexagon_border(border_color, border, border_pivot, p1, p2, p3, p4, p5, p6));
		return composite;
	}

	Polygon2DComposite create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot, const std::vector<glm::vec2>& points)
	{
		Polygon2DComposite composite;
		composite.push_back(create_ngon(fill_color, points));
		composite.push_back(create_ngon_border(border_color, border, border_pivot, points));
		return composite;
	}

	Polygon2DComposite create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, BorderPivot border_pivot, std::vector<glm::vec2>&& points)
	{
		Polygon2DComposite composite;
		TriangulatedPolygon2D ngon_border = create_ngon_border(border_color, border, border_pivot, points);
		composite.push_back(create_ngon(fill_color, std::move(points)));
		composite.push_back(std::move(ngon_border));
		return composite;
	}

	Polygon2DComposite create_bordered_ngon(const std::vector<glm::vec4>& fill_colors, const std::vector<glm::vec4>& border_colors,
		float border, BorderPivot border_pivot, const std::vector<glm::vec2>& points)
	{
		bool solid_border_color = (points.size() == border_colors.size() && border_colors.size() != 1);
		auto ngon = create_bordered_ngon(glm::vec4{}, glm::vec4{}, border, border_pivot, points);
		ngon[0].polygon.colors = fill_colors;
		if (solid_border_color)
		{
			ngon[1].polygon.colors.clear();
			ngon[1].polygon.colors.reserve(border_colors.size() * 2);
			for (size_t i = 0; i < border_colors.size(); ++i)
			{
				ngon[1].polygon.colors.push_back(border_colors[i]);
				ngon[1].polygon.colors.push_back(border_colors[i]);
			}
		}
		else
			ngon[1].polygon.colors = border_colors;
		return ngon;
	}

	Polygon2DComposite create_bordered_ngon(std::vector<glm::vec4>&& fill_colors, std::vector<glm::vec4>&& border_colors,
		float border, BorderPivot border_pivot, std::vector<glm::vec2>&& points)
	{
		bool solid_border_color = (points.size() == border_colors.size() && border_colors.size() != 1);
		auto ngon = create_bordered_ngon(glm::vec4{}, glm::vec4{}, border, border_pivot, std::move(points));
		ngon[0].polygon.colors = std::move(fill_colors);
		if (solid_border_color)
		{
			ngon[1].polygon.colors.clear();
			ngon[1].polygon.colors.reserve(border_colors.size() * 2);
			for (size_t i = 0; i < border_colors.size(); ++i)
			{
				ngon[1].polygon.colors.push_back(border_colors[i]);
				ngon[1].polygon.colors.push_back(border_colors[i]);
			}
		}
		else
			ngon[1].polygon.colors = std::move(border_colors);
		return ngon;
	}

	Polygon2DComposite NGonBase::composite() const
	{
		return { create_ngon(fill_colors, points) };
	}

	Polygon2DComposite NGonBase::composite(glm::uint max_degree) const
	{
		return split_polygon_composite(composite()[0], max_degree);
	}

	Polygon2DComposite NGonBase::bordered_composite() const
	{
		return { create_bordered_ngon(fill_colors, border_colors, border_width, border_pivot, points) };
	}

	Polygon2DComposite NGonBase::bordered_composite(glm::uint max_degree) const
	{
		auto comp = bordered_composite();
		split_polygon_composite(comp, max_degree);
		return comp;
	}
}
