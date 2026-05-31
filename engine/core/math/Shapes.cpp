#include "Shapes.h"

#include "core/math/Geometry.h"
#include "core/types/Approximate.h"
#include "core/base/Transforms.h"
#include "core/util/Parser.h"
#include "core/util/Logger.h"

#include "definitions/Keys.h"

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

	IRect2D IRect2D::load(TOMLNode node, bool validate)
	{
		if (!node)
			return {};

		assets::Parser parser(node);

		IRect2D rect;
		parser.optional(detail::Key::X1)(rect.x1);
		parser.optional(detail::Key::X2)(rect.x2);
		parser.optional(detail::Key::Y1)(rect.y1);
		parser.optional(detail::Key::Y2)(rect.y2);

		if (validate)
		{
			if (rect.x2 <= rect.x1 || rect.y2 <= rect.y1)
			{
				_OLY_ENGINE_LOG_ERROR("CONTEXT") << "cannot parse IRect2D (x1=" << rect.x1 << ";x2=" << rect.x2 << ";y1=" << rect.y1 << ";y2=" << rect.y2 << ") - invalid bounds" << LOG.endl;
				throw Error(ErrorCode::LoadAsset);
			}
		}

		return rect;
	}

	Padding Padding::load(TOMLNode node)
	{
		if (!node)
			return {};

		assets::Parser parser(node);

		Padding padding;

		if (auto uniform = parser.optional<double>(detail::Key::Uniform)())
			padding = Padding::uniform(*uniform);

		parser.optional(detail::Key::Left)(padding.left);
		parser.optional(detail::Key::Right)(padding.right);
		parser.optional(detail::Key::Top)(padding.top);
		parser.optional(detail::Key::Bottom)(padding.bottom);

		return padding;
	}

	TopSidePadding TopSidePadding::load(TOMLNode node)
	{
		if (!node)
			return {};

		assets::Parser parser(node);

		TopSidePadding padding;

		if (auto uniform = parser.optional<double>(detail::Key::Uniform)())
			padding = TopSidePadding::uniform(*uniform);

		parser.optional(detail::Key::Left)(padding.left);
		parser.optional(detail::Key::Right)(padding.right);
		parser.optional(detail::Key::Top)(padding.top);

		return padding;
	}
}
