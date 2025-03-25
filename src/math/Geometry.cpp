#include "Geometry.h"

float oly::math::Triangle2D::signed_area() const
{
	return 0.5f * (root.x * (prev.y - next.y) + prev.x * (next.y - root.y) + next.x * (root.y - prev.y));
}

oly::math::Barycentric oly::math::Triangle2D::barycentric(glm::vec2 point) const
{
	float denom = signed_area();
	Triangle2D tRoot{ point, prev, next };
	Triangle2D tPrev{ point, next, root };
	Triangle2D tNext{ point, root, prev };
	Barycentric b;
	b.root() = tRoot.signed_area() / denom;
	b.prev() = tPrev.signed_area() / denom;
	b.next() = tNext.signed_area() / denom;
	return b;
}

float oly::math::cross(glm::vec2 u, glm::vec2 v)
{
	return u.x * v.y - u.y * v.x;
}

void oly::math::Polygon2D::fill_colors()
{
	if (colors.size() == 1)
		for (size_t i = 1; i < points.size(); ++i)
			colors.push_back(colors[0]);
}

bool oly::math::Polygon2D::valid() const
{
	return points.size() >= 3 && (colors.size() == 1 || colors.size() == points.size());
}

void oly::math::Polygon2D::merge(const Polygon2D& other)
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

size_t oly::math::num_triangulated_indices(const Polygon2D& polygon)
{
	return polygon.points.size() >= 3 ? (polygon.points.size() - 2) * 3 : 0;
}

void oly::math::Triangulation::set_index_offset(glm::uint new_offset)
{
	int diff = (int)new_offset - (int)index_offset;
	index_offset = new_offset;
	for (glm::uvec3& face : faces)
	{
		face[0] = (int)face[0] + diff;
		face[1] = (int)face[1] + diff;
		face[2] = (int)face[2] + diff;
	}
}

std::pair<glm::vec2, glm::vec2> oly::math::border_points(glm::vec2 point, float border, float border_pivot, glm::vec2 prev_point, glm::vec2 next_point)
{
	glm::vec2 v_prev = glm::normalize(prev_point - point);
	glm::vec2 v_next = glm::normalize(next_point - point);
	glm::vec2 n_prev{ v_prev.y, -v_prev.x };
	glm::vec2 n_next{ -v_next.y, v_next.x };
	if (cross(point - prev_point, next_point - point) <= 0.0f)
	{
		n_prev *= -1;
		n_next *= -1;
	}
	float b = border * border_pivot;
	float t = b * glm::length(n_next - n_prev) / glm::length(v_prev - v_next);
	glm::vec2 inner = point + b * n_next + t * v_next;
	b = border * (border_pivot - 1.0f);
	t = b * glm::length(n_next - n_prev) / glm::length(v_prev - v_next);
	glm::vec2 outer = point + b * n_next + t * v_next;
	return { inner, outer };
}

oly::math::TriangulatedPolygon2D oly::math::create_triangle(glm::vec4 color, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
	TriangulatedPolygon2D p;
	p.polygon.colors.push_back(color);
	p.polygon.points.reserve(3);
	p.polygon.points.push_back(p1);
	p.polygon.points.push_back(p2);
	p.polygon.points.push_back(p3);
	p.triangulation = ear_clipping(0, p.polygon);
	return p;
}

oly::math::TriangulatedPolygon2D oly::math::create_triangle_border(glm::vec4 color, float border, float border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
	TriangulatedPolygon2D p;
	p.polygon.colors.push_back(color);
	auto ps1 = border_points(p1, border, border_pivot, p3, p2);
	auto ps2 = border_points(p2, border, border_pivot, p1, p3);
	auto ps3 = border_points(p3, border, border_pivot, p2, p1);
	p.polygon.points.reserve(2 * 3 + 2);
	p.polygon.points.push_back(ps1.second);
	p.polygon.points.push_back(ps2.second);
	p.polygon.points.push_back(ps3.second);
	p.polygon.points.push_back(ps1.second); // connector
	p.polygon.points.push_back(ps1.first); // connector
	p.polygon.points.push_back(ps3.first);
	p.polygon.points.push_back(ps2.first);
	p.polygon.points.push_back(ps1.first);
	p.triangulation = ear_clipping(0, p.polygon);
	p.polygon.points.erase(p.polygon.points.begin() + 3, p.polygon.points.begin() + 5); // remove connectors
	static const auto reindex = [](glm::uint& index) {
		if (index == 3)
			index = 0;
		else if (index == 4)
			index = 5;
		else if (index > 3)
			index -= 2;
		};
	for (glm::uvec3& face : p.triangulation.faces)
	{
		reindex(face.x);
		reindex(face.y);
		reindex(face.z);
	}
	return p;
}

oly::math::TriangulatedPolygon2D oly::math::create_quad(glm::vec4 color, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4)
{
	TriangulatedPolygon2D p;
	p.polygon.colors.push_back(color);
	p.polygon.points.reserve(4);
	p.polygon.points.push_back(p1);
	p.polygon.points.push_back(p2);
	p.polygon.points.push_back(p3);
	p.polygon.points.push_back(p4);
	p.triangulation = ear_clipping(0, p.polygon);
	return p;
}

oly::math::TriangulatedPolygon2D oly::math::create_quad_border(glm::vec4 color, float border, float border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4)
{
	TriangulatedPolygon2D p;
	p.polygon.colors.push_back(color);
	auto ps1 = border_points(p1, border, border_pivot, p4, p2);
	auto ps2 = border_points(p2, border, border_pivot, p1, p3);
	auto ps3 = border_points(p3, border, border_pivot, p2, p4);
	auto ps4 = border_points(p4, border, border_pivot, p3, p1);
	p.polygon.points.reserve(2 * 4 + 2);
	p.polygon.points.push_back(ps1.second);
	p.polygon.points.push_back(ps2.second);
	p.polygon.points.push_back(ps3.second);
	p.polygon.points.push_back(ps4.second);
	p.polygon.points.push_back(ps1.second); // connector
	p.polygon.points.push_back(ps1.first); // connector
	p.polygon.points.push_back(ps4.first);
	p.polygon.points.push_back(ps3.first);
	p.polygon.points.push_back(ps2.first);
	p.polygon.points.push_back(ps1.first);
	p.triangulation = ear_clipping(0, p.polygon);
	p.polygon.points.erase(p.polygon.points.begin() + 4, p.polygon.points.begin() + 6); // remove connectors
	static const auto reindex = [](glm::uint& index) {
		if (index == 4)
			index = 0;
		else if (index == 5)
			index = 7;
		else if (index > 4)
			index -= 2;
		};
	for (glm::uvec3& face : p.triangulation.faces)
	{
		reindex(face.x);
		reindex(face.y);
		reindex(face.z);
	}
	return p;
}

oly::math::TriangulatedPolygon2D oly::math::create_pentagon(glm::vec4 color, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5)
{
	TriangulatedPolygon2D p;
	p.polygon.colors.push_back(color);
	p.polygon.points.reserve(5);
	p.polygon.points.push_back(p1);
	p.polygon.points.push_back(p2);
	p.polygon.points.push_back(p3);
	p.polygon.points.push_back(p4);
	p.polygon.points.push_back(p5);
	p.triangulation = ear_clipping(0, p.polygon);
	return p;
}

oly::math::TriangulatedPolygon2D oly::math::create_pentagon_border(glm::vec4 color, float border, float border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5)
{
	TriangulatedPolygon2D p;
	p.polygon.colors.push_back(color);
	auto ps1 = border_points(p1, border, border_pivot, p5, p2);
	auto ps2 = border_points(p2, border, border_pivot, p1, p3);
	auto ps3 = border_points(p3, border, border_pivot, p2, p4);
	auto ps4 = border_points(p4, border, border_pivot, p3, p5);
	auto ps5 = border_points(p5, border, border_pivot, p4, p1);
	p.polygon.points.reserve(2 * 5 + 2);
	p.polygon.points.push_back(ps1.second);
	p.polygon.points.push_back(ps2.second);
	p.polygon.points.push_back(ps3.second);
	p.polygon.points.push_back(ps4.second);
	p.polygon.points.push_back(ps5.second);
	p.polygon.points.push_back(ps1.second); // connector
	p.polygon.points.push_back(ps1.first); // connector
	p.polygon.points.push_back(ps5.first);
	p.polygon.points.push_back(ps4.first);
	p.polygon.points.push_back(ps3.first);
	p.polygon.points.push_back(ps2.first);
	p.polygon.points.push_back(ps1.first);
	p.triangulation = ear_clipping(0, p.polygon);
	p.polygon.points.erase(p.polygon.points.begin() + 5, p.polygon.points.begin() + 7); // remove connectors
	static const auto reindex = [](glm::uint& index) {
		if (index == 5)
			index = 0;
		else if (index == 6)
			index = 9;
		else if (index > 5)
			index -= 2;
		};
	for (glm::uvec3& face : p.triangulation.faces)
	{
		reindex(face.x);
		reindex(face.y);
		reindex(face.z);
	}
	return p;
}

oly::math::TriangulatedPolygon2D oly::math::create_hexagon(glm::vec4 color, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6)
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
	p.triangulation = ear_clipping(0, p.polygon);
	return p;
}

oly::math::TriangulatedPolygon2D oly::math::create_hexagon_border(glm::vec4 color, float border, float border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6)
{
	TriangulatedPolygon2D p;
	p.polygon.colors.push_back(color);
	auto ps1 = border_points(p1, border, border_pivot, p6, p2);
	auto ps2 = border_points(p2, border, border_pivot, p1, p3);
	auto ps3 = border_points(p3, border, border_pivot, p2, p4);
	auto ps4 = border_points(p4, border, border_pivot, p3, p5);
	auto ps5 = border_points(p5, border, border_pivot, p4, p6);
	auto ps6 = border_points(p6, border, border_pivot, p5, p1);
	p.polygon.points.reserve(2 * 6 + 2);
	p.polygon.points.push_back(ps1.second);
	p.polygon.points.push_back(ps2.second);
	p.polygon.points.push_back(ps3.second);
	p.polygon.points.push_back(ps4.second);
	p.polygon.points.push_back(ps5.second);
	p.polygon.points.push_back(ps6.second);
	p.polygon.points.push_back(ps1.second); // connector
	p.polygon.points.push_back(ps1.first); // connector
	p.polygon.points.push_back(ps6.first);
	p.polygon.points.push_back(ps5.first);
	p.polygon.points.push_back(ps4.first);
	p.polygon.points.push_back(ps3.first);
	p.polygon.points.push_back(ps2.first);
	p.polygon.points.push_back(ps1.first);
	p.triangulation = ear_clipping(0, p.polygon);
	p.polygon.points.erase(p.polygon.points.begin() + 6, p.polygon.points.begin() + 8); // remove connectors
	static const auto reindex = [](glm::uint& index) {
		if (index == 6)
			index = 0;
		else if (index == 7)
			index = 11;
		else if (index > 6)
			index -= 2;
		};
	for (glm::uvec3& face : p.triangulation.faces)
	{
		reindex(face.x);
		reindex(face.y);
		reindex(face.z);
	}
	return p;
}

oly::math::TriangulatedPolygon2D oly::math::create_ngon(glm::vec4 color, const std::vector<glm::vec2>& points)
{
	assert(points.size() >= 3);
	TriangulatedPolygon2D p;
	p.polygon.colors.push_back(color);
	p.polygon.points.reserve(points.size());
	for (glm::vec2 pt : points)
		p.polygon.points.push_back(pt);
	p.triangulation = ear_clipping(0, p.polygon);
	return p;
}

oly::math::TriangulatedPolygon2D oly::math::create_ngon_border(glm::vec4 color, float border, float border_pivot, const std::vector<glm::vec2>& points)
{
	size_t num_points = points.size();
	assert(num_points >= 3);
	TriangulatedPolygon2D p;
	p.polygon.colors.push_back(color);
	std::vector<std::pair<glm::vec2, glm::vec2>> bpts;
	bpts.reserve(num_points);
	bpts.push_back(border_points(points[0], border, border_pivot, points.back(), points[1]));
	for (size_t i = 1; i < num_points - 1; ++i)
		bpts.push_back(border_points(points[i], border, border_pivot, points[i - 1], points[i + 1]));
	bpts.push_back(border_points(points.back(), border, border_pivot, points[num_points - 2], points[0]));
	p.polygon.points.reserve(2 * num_points + 2);
	for (auto iter = bpts.begin(); iter != bpts.end(); ++iter)
		p.polygon.points.push_back(iter->second);
	p.polygon.points.push_back(bpts[0].second); // connector
	p.polygon.points.push_back(bpts[0].first); // connector
	for (auto iter = bpts.rbegin(); iter != bpts.rend(); ++iter)
		p.polygon.points.push_back(iter->first);
	p.triangulation = ear_clipping(0, p.polygon);
	p.polygon.points.erase(p.polygon.points.begin() + num_points, p.polygon.points.begin() + num_points + 2); // remove connectors
	static const auto reindex = [](glm::uint& index, size_t size) {
		if (index == size)
			index = 0;
		else if (index == size + 1)
			index = 2 * size - 1;
		else if (index > size)
			index -= 2;
		};
	for (glm::uvec3& face : p.triangulation.faces)
	{
		reindex(face.x, num_points);
		reindex(face.y, num_points);
		reindex(face.z, num_points);
	}
	return p;
}

oly::math::Polygon2DComposite oly::math::split_polygon_composite(const TriangulatedPolygon2D& tp, glm::uint max_degree)
{
	// TODO
	return Polygon2DComposite();
}

void oly::math::split_polygon_composite(Polygon2DComposite& composite, glm::uint max_degree)
{
	// TODO
}

oly::math::Polygon2DComposite oly::math::create_bordered_triangle(glm::vec4 fill_color, glm::vec4 border_color, float border, float border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
	Polygon2DComposite composite;
	composite.push_back(create_triangle(fill_color, p1, p2, p3));
	composite.push_back(create_triangle_border(border_color, border, border_pivot, p1, p2, p3));
	return composite;
}

oly::math::Polygon2DComposite oly::math::create_bordered_quad(glm::vec4 fill_color, glm::vec4 border_color, float border, float border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4)
{
	Polygon2DComposite composite;
	composite.push_back(create_quad(fill_color, p1, p2, p3, p4));
	composite.push_back(create_quad_border(border_color, border, border_pivot, p1, p2, p3, p4));
	return composite;
}

oly::math::Polygon2DComposite oly::math::create_bordered_pentagon(glm::vec4 fill_color, glm::vec4 border_color, float border, float border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5)
{
	Polygon2DComposite composite;
	composite.push_back(create_pentagon(fill_color, p1, p2, p3, p4, p5));
	composite.push_back(create_pentagon_border(border_color, border, border_pivot, p1, p2, p3, p4, p5));
	return composite;
}

oly::math::Polygon2DComposite oly::math::create_bordered_hexagon(glm::vec4 fill_color, glm::vec4 border_color, float border, float border_pivot, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6)
{
	Polygon2DComposite composite;
	composite.push_back(create_hexagon(fill_color, p1, p2, p3, p4, p5, p6));
	composite.push_back(create_hexagon_border(border_color, border, border_pivot, p1, p2, p3, p4, p5, p6));
	return composite;
}

oly::math::Polygon2DComposite oly::math::create_bordered_ngon(glm::vec4 fill_color, glm::vec4 border_color, float border, float border_pivot, const std::vector<glm::vec2>& points)
{
	Polygon2DComposite composite;
	composite.push_back(create_ngon(fill_color, points));
	composite.push_back(create_ngon_border(border_color, border, border_pivot, points));
	return composite;
}
