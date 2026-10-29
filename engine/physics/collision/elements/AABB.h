#pragma once

#include "core/base/Constants.h"
#include "core/base/SimpleMath.h"
#include "core/base/UnitVector.h"

#include "core/math/Geometry.h"

#include "physics/collision/Tolerance.h"

#include "physics/collision/methods/CollisionInfo.h"

#include <array>

namespace oly::math
{
    struct Rect2D;
}

namespace oly::col2d
{
    struct AABB
    {
        static const AABB kDefault;

        float x1;
        float x2;
        float y1;
        float y2;

        static AABB wrap(const glm::vec2* polygon, size_t count);

        float area() const;
        glm::vec2 center() const;
        float width() const;
        float height() const;

        std::array<glm::vec2, 4> points() const;
        fpair projection_interval(const UnitVector2D& axis) const;
        float projection_min(const UnitVector2D& axis) const;
        float projection_max(const UnitVector2D& axis) const;
        ContactManifold deepest_manifold(const UnitVector2D& axis) const;
        math::Rect2D rect() const;
    };

    inline const AABB AABB::kDefault = AABB{
        .x1 = nmax<float>(),
        .x2 = -nmax<float>(),
        .y1 = nmax<float>(),
        .y2 = -nmax<float>()
    };;
}
