#include "Coordinates.h"

namespace oly::math
{
	Barycentric::Barycentric(const Triangle2D& triangle, glm::vec2 point)
	{
		float denom = triangle.signed_area();
		Triangle2D tRoot{ point, triangle.prev, triangle.next };
		Triangle2D tPrev{ point, triangle.next, triangle.root };
		Triangle2D tNext{ point, triangle.root, triangle.prev };
		root() = tRoot.signed_area() / denom;
		prev() = tPrev.signed_area() / denom;
		next() = tNext.signed_area() / denom;
	}
}
