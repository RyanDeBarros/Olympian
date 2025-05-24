#pragma once

#include "physics/collision/abstract/compound/Compound.h"

namespace oly::acm2d
{
	struct Capsule
	{
		glm::vec2 center;
		float half_width, half_height, rotation;

		glm::mat2 get_rotation_matrix() const
		{
			float cos = glm::cos(rotation);
			float sin = glm::sin(rotation);
			return { { cos, sin }, { -sin, cos } };
		}

		OBB mid_obb() const { return { .center = center, .width = 2 * half_width, .height = 2 * half_height, .rotation = rotation }; }
		Circle upper_circle() const { return { .center = center + get_rotation_matrix() * glm::vec2{ 0.0f, half_height }, .radius = half_width }; }
		Circle lower_circle() const { return { .center = center + get_rotation_matrix() * glm::vec2{ 0.0f, -half_height }, .radius = half_width }; }

		UnitVector2D horizontal() const { return UnitVector2D(rotation); }
		UnitVector2D vertical() const { return UnitVector2D(rotation + glm::half_pi<float>()); }

		Compound compound() const { return { { lower_circle(), mid_obb(), upper_circle() } }; }
	};
}
