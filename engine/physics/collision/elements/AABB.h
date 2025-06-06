#pragma once

#include "core/base/UnitVector.h"
#include "core/base/Constants.h"
#include "core/math/Geometry.h"

#include <array>

namespace oly::col2d
{
	struct AABB
	{
		float x1, x2, y1, y2;

		float area() const { return width() * height(); }
		glm::vec2 center() const { return 0.5f * glm::vec2{ x1 + x2, y1 + y2 }; }
		float width() const { return x2 - x1; }
		float height() const { return y2 - y1; }

		static AABB wrap(const glm::vec2* polygon, size_t count);
		static const AABB DEFAULT;

		std::array<glm::vec2, 4> points() const;
		std::pair<float, float> projection_interval(const UnitVector2D& axis) const;
		glm::vec2 deepest_point(const UnitVector2D& axis) const;
	};

	inline const AABB AABB::DEFAULT = AABB{ .x1 = nmax<float>(), .x2 = -nmax<float>(), .y1 = nmax<float>(), .y2 = -nmax<float>() };;
}
