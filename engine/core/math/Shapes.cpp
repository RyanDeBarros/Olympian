#include "Shapes.h"

#include "core/math/Geometry.h"
#include "core/types/Approximate.h"
#include "core/base/Transforms.h"
#include "core/util/Parser.h"

#include ".gen/keys/Rect2D.inl"
#include ".gen/keys/Padding.inl"

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

		io::Parser parser(node);

		IRect2D rect;
		parser.optional(_gen::keys::Rect2D::X1)(rect.x1);
		parser.optional(_gen::keys::Rect2D::X2)(rect.x2);
		parser.optional(_gen::keys::Rect2D::Y1)(rect.y1);
		parser.optional(_gen::keys::Rect2D::Y2)(rect.y2);
		return rect;
	}

	Padding Padding::load(TOMLNode node)
	{
		if (!node)
			return {};

		io::Parser parser(node);

		Padding padding;

		if (auto uniform = parser.optional<double>(_gen::keys::Padding::Uniform)())
			padding = Padding::uniform(*uniform);

		parser.optional(_gen::keys::Padding::Left)(padding.left);
		parser.optional(_gen::keys::Padding::Right)(padding.right);
		parser.optional(_gen::keys::Padding::Top)(padding.top);
		parser.optional(_gen::keys::Padding::Bottom)(padding.bottom);

		return padding;
	}

	TopSidePadding TopSidePadding::load(TOMLNode node)
	{
		if (!node)
			return {};

		io::Parser parser(node);

		TopSidePadding padding;

		if (auto uniform = parser.optional<double>(_gen::keys::Padding::Uniform)())
			padding = TopSidePadding::uniform(*uniform);

		parser.optional(_gen::keys::Padding::Left)(padding.left);
		parser.optional(_gen::keys::Padding::Right)(padding.right);
		parser.optional(_gen::keys::Padding::Top)(padding.top);

		return padding;
	}
}
