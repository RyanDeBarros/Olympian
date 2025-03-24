#include "Geometry.h"

float oly::math::Triangle2D::signed_area() const
{
	return 0.5f * (root.x * (prev.y - next.y) + prev.x * (next.y - root.y) + next.x * (root.y - prev.y));
}

oly::math::Barycentric oly::math::Triangle2D::barycentric(glm::vec2 point) const
{
	float denom = area();
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

oly::math::Polygon2D oly::math::Polygon2D::create_quad(glm::vec4 color, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4)
{
	Polygon2D p;
	p.colors.push_back(color);
	p.points.reserve(4);
	p.points.push_back(p1);
	p.points.push_back(p2);
	p.points.push_back(p3);
	p.points.push_back(p4);
	return p;
}

oly::math::Polygon2D oly::math::Polygon2D::create_pentagon(glm::vec4 color, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5)
{
	Polygon2D p;
	p.colors.push_back(color);
	p.points.reserve(5);
	p.points.push_back(p1);
	p.points.push_back(p2);
	p.points.push_back(p3);
	p.points.push_back(p4);
	p.points.push_back(p5);
	return p;
}

oly::math::Polygon2D oly::math::Polygon2D::create_hexagon(glm::vec4 color, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4, glm::vec2 p5, glm::vec2 p6)
{
	Polygon2D p;
	p.colors.push_back(color);
	p.points.reserve(6);
	p.points.push_back(p1);
	p.points.push_back(p2);
	p.points.push_back(p3);
	p.points.push_back(p4);
	p.points.push_back(p5);
	p.points.push_back(p6);
	return p;
}

size_t oly::math::num_triangulated_indices(const Polygon2D& polygon)
{
	return polygon.points.size() >= 3 ? (polygon.points.size() - 2) * 3 : 0;
}
