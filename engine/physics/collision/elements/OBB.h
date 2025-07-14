#pragma once

#include "core/base/UnitVector.h"
#include "core/base/SimpleMath.h"
#include "core/math/Geometry.h"
#include "physics/collision/elements/AABB.h"
#include "physics/collision/Tolerance.h"
#include "physics/collision/methods/CollisionInfo.h"

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
		static OBB slow_wrap(const glm::vec2* polygon, size_t count);

		UnitVector2D get_major_axis() const { return UnitVector2D(rotation); }
		UnitVector2D get_minor_axis() const { return UnitVector2D(rotation).get_quarter_turn(); }
		fpair get_major_axis_projection_interval() const
		{
			float c = UnitVector2D(rotation).dot(center);
			return { c - 0.5f * width, c + 0.5f * width };
		}
		fpair get_minor_axis_projection_interval() const
		{
			float c = UnitVector2D(rotation).get_quarter_turn().dot(center);
			return { c - 0.5f * height, c + 0.5f * height };
		}

		glm::mat2 get_rotation_matrix() const
		{
			float cos = glm::cos(rotation);
			float sin = glm::sin(rotation);
			return { { cos, sin }, { -sin, cos } };
		}

		glm::mat2 get_inv_rotation_matrix() const
		{
			float cos = glm::cos(rotation);
			float sin = -glm::sin(rotation);
			return { { cos, sin }, { -sin, cos } };
		}

		std::array<glm::vec2, 4> points() const;
		fpair projection_interval(const UnitVector2D& axis) const;
		float projection_min(const UnitVector2D& axis) const;
		float projection_max(const UnitVector2D& axis) const;
		ContactManifold deepest_manifold(const UnitVector2D& axis) const;
	};
}
