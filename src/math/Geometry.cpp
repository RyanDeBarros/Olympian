#include "Geometry.h"

float oly::math::Triangle2D::area() const
{
	return 0.5f * std::abs(root.x * (prev.y - next.y) + prev.x * (next.y - root.y) + next.x * (root.y - prev.y));
}

oly::math::Barycentric oly::math::Triangle2D::barycentric(glm::vec2 point) const
{
	float denom = area();
	Triangle2D tRoot{ point, prev, next };
	Triangle2D tPrev{ point, next, root };
	Triangle2D tNext{ point, root, prev };
	Barycentric b;
	b.root() = tRoot.area() / denom;
	b.prev() = tPrev.area() / denom;
	b.next() = tNext.area() / denom;
	return b;
}

float oly::math::cross(glm::vec2 u, glm::vec2 v)
{
	return u.x * v.y - u.y * v.x;
}

size_t oly::math::num_triangulated_faces(const Polygon2D& polygon)
{
	return polygon.points.size() >= 3 ? (polygon.points.size() - 2) * 3 : 0;
}
