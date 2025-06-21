#pragma once

#include "core/base/UnitVector.h"
#include "physics/collision/Tolerance.h"

namespace oly::col2d
{
	struct Capsule
	{
		glm::vec2 center = {};
		float obb_width = 0.0f, obb_height = 0.0f, rotation = 0.0f;

		glm::mat2 get_rotation_matrix() const
		{
			float cos = glm::cos(rotation);
			float sin = glm::sin(rotation);
			return { { cos, sin }, { -sin, cos } };
		}

		OBB mid_obb() const { return { .center = center, .width = obb_width, .height = obb_height, .rotation = rotation }; }
		Circle lower_circle() const { return Circle(center + get_rotation_matrix() * glm::vec2{ 0.0f, -0.5f * obb_height }, 0.5f * obb_width); }
		Circle upper_circle() const { return Circle(center + get_rotation_matrix() * glm::vec2{ 0.0f,  0.5f * obb_height }, 0.5f * obb_width); }

		UnitVector2D horizontal() const { return UnitVector2D(rotation); }
		UnitVector2D vertical() const { return UnitVector2D(rotation + glm::half_pi<float>()); }

		Compound compound() const { return { { mid_obb(), lower_circle(), upper_circle() } }; }
		TCompound tcompound() const { return TCompound({ mid_obb(), lower_circle(), upper_circle() }); }
		template<typename Shape = OBB>
		BVH<Shape> bvh() const { return convert<Shape>(compound()); }
		template<typename Shape = OBB>
		TBVH<Shape> tbvh() const { return convert<Shape>(tcompound()); }
	};
}
