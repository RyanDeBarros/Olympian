#include "CircleMethods.h"

namespace oly::col2d::internal
{
    bool circle_intersects_edge(const Circle& c, const UnitVector2D& edge, glm::vec2 a, glm::vec2 b)
    {
        float cross = math::cross(edge, c.center - a);
        float discriminant = c.radius * c.radius - cross * cross;
        if (discriminant >= 0.0f)
        {
            float offset = edge.dot(c.center - a);
            discriminant = glm::sqrt(discriminant);
            float t1 = offset - discriminant;
            float t2 = offset + discriminant;

            if (t2 >= 0.0f && t1 <= glm::length(b - a))
                return true;
        }
        return false;
    };
}
