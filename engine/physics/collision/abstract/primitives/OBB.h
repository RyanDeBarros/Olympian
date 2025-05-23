#pragma once

#include "core/base/UnitVector.h"
#include "core/math/Geometry.h"
#include "physics/collision/abstract/primitives/AABB.h"

#include <array>

namespace oly::acm2d
{
	struct OBB
	{
		glm::vec2 center;
		float width, height;
		float rotation;

		float area() const { return width * height; }

		static OBB fast_wrap(const math::Polygon2D& polygon);
		// TODO static OBB slow_wrap(const math::Polygon2D& polygon); using rotating calipers method

		std::array<UnitVector2D, 2> get_axes() const
		{
			return { UnitVector2D(rotation), UnitVector2D(rotation + glm::half_pi<float>()) };
		}

		glm::mat2 get_rotation_matrix() const
		{
			float cos = glm::cos(rotation);
			float sin = glm::sin(rotation);
			return { { cos, sin }, { -sin, cos } };
		}

		std::array<glm::vec2, 4> points() const;
		std::pair<float, float> projection_interval(const UnitVector2D& axis) const;
		AABB get_unrotated_aabb() const;
		glm::vec2 deepest_point(const UnitVector2D& axis) const;
	};
}
