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
		static OBB wrap_axis_aligned(const math::Polygon2D& polygon, float rotation);
		// TODO static OBB slow_wrap(const math::Polygon2D& polygon); using rotating calipers method

		UnitVector2D get_axis_1() const { return UnitVector2D(rotation); }
		float get_axis_1_projection() const { return UnitVector2D(rotation).dot(center); }

		std::pair<float, float> get_axis_1_projection_interval() const
		{
			float x = get_axis_1_projection();
			return { x - 0.5f * width, x + 0.5f * width };
		}

		UnitVector2D get_axis_2() const { return UnitVector2D(rotation + glm::half_pi<float>()); }
		float get_axis_2_projection() const { return UnitVector2D(rotation + glm::half_pi<float>()).dot(center); }

		std::pair<float, float> get_axis_2_projection_interval() const
		{
			float y = get_axis_2_projection();
			return { y - 0.5f * height, y + 0.5f * height };
		}

		glm::mat2 get_rotation_matrix() const
		{
			float cos = glm::cos(rotation);
			float sin = glm::sin(rotation);
			return { { cos, sin }, { -sin, cos } };
		}

		std::array<glm::vec2, 4> points() const;
		std::pair<float, float> projection_interval(const UnitVector2D& axis) const;
		glm::vec2 deepest_point(const UnitVector2D& axis) const;
	};
}
