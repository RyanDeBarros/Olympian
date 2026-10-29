#pragma once

#include "physics/collision/Tolerance.h"

#include "physics/collision/methods/Collide.h"

#include "physics/collision/objects/Capsule.h"

namespace oly::col2d
{
    struct RectCast
    {
        Ray ray;
        float width;
        float depth;

        OBB finite_obb(float infinite_clip = 0.0f) const;
    };

    struct CircleCast
    {
        Ray ray;
        float radius;

        Capsule finite_capsule(float infinite_clip = 0.0f) const;
    };

    template<typename Shape>
    inline OverlapResult rect_cast_hits(const Shape& c, const RectCast& cast)
    {
        if (cast.ray.clip == 0.0f)
        {
            RectCast finite = cast;
            finite.ray.clip = std::max(
                c.projection_max(cast.ray.direction) - cast.ray.direction.dot(cast.ray.origin),
                0.0f);

            return overlaps(c, finite.finite_obb());
        }
        else
            return overlaps(c, cast.finite_obb());
    }

    template<typename Shape>
    inline OverlapResult circle_cast_hits(const Shape& c, const CircleCast& cast)
    {
        if (cast.ray.clip == 0.0f)
        {
            CircleCast finite = cast;
            finite.ray.clip = std::max(
                c.projection_max(cast.ray.direction) - cast.ray.direction.dot(cast.ray.origin),
                0.0f);

            return overlaps(c, finite.finite_capsule().compound());
        }
        else
            return overlaps(c, cast.finite_capsule().compound());
    }
}
