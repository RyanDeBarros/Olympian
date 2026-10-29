#pragma once

#include "core/math/Shapes.h"

namespace oly::math
{
    namespace coordinates
    {
        extern glm::vec2 to_polar(glm::vec2 cartesian);
        extern glm::vec2 to_cartesian(glm::vec2 polar);
        extern glm::vec3 to_spherical(glm::vec3 cartesian);
        extern glm::vec3 to_cartesian(glm::vec3 spherical);
    }

    struct Barycentric
        : public glm::vec3
    {
        using glm::vec3::vec;

        Barycentric(const Triangle2D& triangle, glm::vec2 point);

        float root() const;
        float& root();
        float prev() const;
        float& prev();
        float next() const;
        float& next();

        bool inside() const;
        glm::vec2 point(const Triangle2D& triangle) const;
        glm::vec2 point(glm::vec2 a, glm::vec2 b, glm::vec2 c) const;
    };
}
