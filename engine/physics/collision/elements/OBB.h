#pragma once

#include "core/base/SimpleMath.h"
#include "core/base/UnitVector.h"

#include "core/math/Geometry.h"
#include "core/math/Shapes.h"

#include "physics/collision/Tolerance.h"

#include "physics/collision/elements/AABB.h"

#include "physics/collision/methods/CollisionInfo.h"

#include <array>

namespace oly::col2d
{
    struct OBB
    {
        glm::vec2 center;
        float width;
        float height;
        float rotation;

        static OBB fast_wrap(const glm::vec2* polygon, size_t count);
        static OBB wrap_axis_aligned(const glm::vec2* polygon, size_t count, float rotation);
        static OBB slow_wrap(const glm::vec2* polygon, size_t count);

        UnitVector2D get_major_axis() const;
        UnitVector2D get_minor_axis() const;

        fpair get_major_axis_projection_interval() const;
        fpair get_minor_axis_projection_interval() const;

        glm::mat2 get_rotation_matrix() const;
        glm::mat2 get_inv_rotation_matrix() const;

        std::array<glm::vec2, 4> points() const;
        math::RotatedRect2D rect() const;
        float area() const;

        fpair projection_interval(const UnitVector2D& axis) const;
        float projection_min(const UnitVector2D& axis) const;
        float projection_max(const UnitVector2D& axis) const;
        ContactManifold deepest_manifold(const UnitVector2D& axis) const;
    };
}
