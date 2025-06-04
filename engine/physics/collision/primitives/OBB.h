#pragma once

#include "core/base/UnitVector.h"
#include "core/math/Geometry.h"
#include "physics/collision/primitives/AABB.h"

#include <array>

namespace oly::col2d
{
	struct OBB
	{
		glm::vec2 center;
		float width, height;
		float rotation;

		float area() const { return width * height; }

		static OBB fast_wrap(const glm::vec2* polygon, size_t count);
		static OBB wrap_axis_aligned(const glm::vec2* polygon, size_t count, float rotation);
		// TODO static OBB slow_wrap(const glm::vec2* polygon, size_t count); using rotating calipers method

		UnitVector2D get_major_axis() const { return UnitVector2D(rotation); }
		UnitVector2D get_minor_axis() const { return UnitVector2D(rotation + glm::half_pi<float>()); }

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
