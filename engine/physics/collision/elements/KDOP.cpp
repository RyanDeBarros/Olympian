#include "KDOP.h"

namespace oly::col2d
{
    namespace internal
    {
        math::Polygon2D initial_kdop_polygon(
            const UnitVector2D& axis0,
            float min0,
            float max0,
            const UnitVector2D& axis1,
            float min1,
            float max1)
        {
            if (axis0.near_parallel(axis1, kLinearTolerance))
                throw Error(ErrorCode::BadCollisionShape, "First 2 kDOP axes are parallel");

            static const auto intersection = [](
                const UnitVector2D& axis0,
                float m0,
                const UnitVector2D& axis1,
                float m1) -> glm::vec2
            {
                glm::mat2 A = { { axis0.x(), axis1.x() }, { axis0.y(), axis1.y() } };
                return glm::inverse(A) * glm::vec2{ m0, m1 };
            };

            return {
                intersection(axis0, min0, axis1, min1),
                intersection(axis0, max0, axis1, min1),
                intersection(axis0, max0, axis1, max1),
                intersection(axis0, min0, axis1, max1)
            };
        }
    }
}
