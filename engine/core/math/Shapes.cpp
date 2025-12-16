#include "Shapes.h"

#include "core/math/Geometry.h"
#include "core/types/Approximate.h"
#include "core/base/Transforms.h"
#include "assets/Loader.h"

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

	IRect2D IRect2D::load(TOMLNode node)
	{
		if (!node)
			return {};

		IRect2D rect;
		assets::parse_int(node["x1"], rect.x1);
		assets::parse_int(node["x2"], rect.x2);
		assets::parse_int(node["y1"], rect.y1);
		assets::parse_int(node["y2"], rect.y2);
		return rect;
	}

	Padding Padding::load(TOMLNode node)
	{
		if (!node)
			return {};

		Padding padding;

		if (auto uniform = node["uniform"].value<double>())
			padding = Padding::uniform(*uniform);

		assets::parse_float(node["left"], padding.left);
		assets::parse_float(node["right"], padding.right);
		assets::parse_float(node["top"], padding.top);
		assets::parse_float(node["bottom"], padding.bottom);

		return padding;
	}

	TopSidePadding TopSidePadding::load(TOMLNode node)
	{
		if (!node)
			return {};

		TopSidePadding padding;

		if (auto uniform = node["uniform"].value<double>())
			padding = TopSidePadding::uniform(*uniform);

		assets::parse_float(node["left"], padding.left);
		assets::parse_float(node["right"], padding.right);
		assets::parse_float(node["top"], padding.top);

		return padding;
	}
}
