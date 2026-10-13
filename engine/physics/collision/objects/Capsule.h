#pragma once

#include "core/base/UnitVector.h"

#include "physics/collision/Tolerance.h"

#include "physics/collision/objects/BVH.h"
#include "physics/collision/objects/Compound.h"

namespace oly::col2d
{
	struct Capsule
	{
		glm::vec2 center = {};
        float obb_width = 0.f;
        float obb_height = 0.f;
        float rotation = 0.f;

        glm::mat2 get_rotation_matrix() const;

		OBB mid_obb() const;
		Circle lower_circle() const;
		Circle upper_circle() const;

        UnitVector2D horizontal() const;
        UnitVector2D vertical() const;

        Compound compound() const;
        TCompound tcompound() const;

		template<typename Shape = OBB>
		BVH<Shape> bvh() const { return convert<Shape>(compound()); }
		
        template<typename Shape = OBB>
		TBVH<Shape> tbvh() const { return convert<Shape>(tcompound()); }
	};
}
