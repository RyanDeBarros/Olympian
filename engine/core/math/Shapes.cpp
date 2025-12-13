#include "Shapes.h"

#include "core/math/Geometry.h"
#include "core/types/Approximate.h"
#include "core/base/Transforms.h"

namespace oly::math
{
	float Triangle2D::signed_area() const
	{
		return 0.5f * (root.x * (prev.y - next.y) + prev.x * (next.y - root.y) + next.x * (root.y - prev.y));
	}

	float Triangle2D::cross() const
	{
		return math::cross(root - prev, next - root);
	}

	bool DirectedLine2D::intersect(const DirectedLine2D& other, glm::vec2 pt) const
	{
		glm::mat2 D = { dir, -other.dir };
		if (near_zero(glm::determinant(D)))
			return false;

		glm::mat2 Dinv = glm::inverse(D);
		glm::vec2 P = other.anchor - anchor;
		glm::vec2 T = Dinv * P;
		pt = anchor + T.x * dir;

		return true;
	}

	std::array<glm::vec2, 4> RotatedRect2D::points() const
	{
		std::array<glm::vec2, 4> points{
			glm::vec2{ -0.5f * size.x, -0.5f * size.y },
			glm::vec2{  0.5f * size.x, -0.5f * size.y },
			glm::vec2{  0.5f * size.x,  0.5f * size.y },
			glm::vec2{ -0.5f * size.x,  0.5f * size.y }
		};
		glm::mat2 rot = rotation_matrix_2x2(rotation);
		for (glm::vec2& point : points)
			point = center + rot * point;
		return points;
	}
}
