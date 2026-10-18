#include "Coordinates.h"

namespace oly::math
{
    namespace coordinates
    {
        glm::vec2 to_polar(glm::vec2 cartesian)
        {
            return { glm::length(cartesian), glm::atan(cartesian.y, cartesian.x) };
        }

        glm::vec2 to_cartesian(glm::vec2 polar)
        {
            return polar.x * glm::vec2{ glm::cos(polar.y), glm::sin(polar.y) };
        }

        glm::vec3 to_spherical(glm::vec3 cartesian)
        {
            const float r = glm::length(cartesian);
            return r != 0.f
                ? glm::vec3(r, glm::atan(cartesian.y, cartesian.x), glm::acos(cartesian.z / r))
                : glm::vec3(0.f);
        }

        glm::vec3 to_cartesian(glm::vec3 spherical)
        {
            const float sphi = glm::sin(spherical.z);
            return spherical.x * glm::vec3{
                glm::cos(spherical.y) * sphi,
                glm::sin(spherical.y) * sphi,
                glm::cosh(spherical.z) };
        }
    }

    Barycentric::Barycentric(const Triangle2D& triangle, glm::vec2 point)
    {
        float denom = triangle.signed_area();
        Triangle2D tRoot{ point, triangle.prev, triangle.next };
        Triangle2D tPrev{ point, triangle.next, triangle.root };
        Triangle2D tNext{ point, triangle.root, triangle.prev };
        root() = tRoot.signed_area() / denom;
        prev() = tPrev.signed_area() / denom;
        next() = tNext.signed_area() / denom;
    }

    float Barycentric::root() const
    {
        return x;
    }

    float& Barycentric::root()
    {
        return x;
    }

    float Barycentric::prev() const
    {
        return y;
    }

    float& Barycentric::prev()
    {
        return y;
    }

    float Barycentric::next() const
    {
        return z;
    }

    float& Barycentric::next()
    {
        return z;
    }

    bool Barycentric::inside() const
    {
        return x >= 0.0f && y >= 0.0f && z >= 0.0f;
    }
    
    glm::vec2 Barycentric::point(const Triangle2D& triangle) const
    {
        return x * triangle.root + y * triangle.prev + z * triangle.next;
    }
    
    glm::vec2 Barycentric::point(glm::vec2 a, glm::vec2 b, glm::vec2 c) const
    {
        return x * a + y * b + z * c;
    }
}
