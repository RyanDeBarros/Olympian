#include "GJK.h"

namespace oly::col2d::gjk
{
    bool handle_simplex(std::vector<glm::vec2>& simplex, UnitVector2D& axis)
    {
        if (simplex.size() == 2)
        {
            // line segment case
            glm::vec2 a = simplex[1];
            glm::vec2 b = simplex[0];

            // dir is orthogonal to (b - a) pointing towards origin
            glm::vec2 dir = math::triple_cross(b - a, -a, b - a);
            if (near_zero(dir))
                axis = UnitVector2D(b - a).get_quarter_turn();
            else
                axis = dir;
        }
        else if (simplex.size() == 3)
        {
            // triangle case
            glm::vec2 a = simplex[2];
            glm::vec2 b = simplex[1];
            glm::vec2 c = simplex[0];

            glm::vec2 ab_ortho = math::triple_cross(c - a, b - a, b - a);
            if (glm::dot(ab_ortho, -a) > 0.0f)
            {
                // remove c
                simplex.erase(simplex.begin());
                axis = ab_ortho;
                return false;
            }

            glm::vec2 ac_ortho = math::triple_cross(b - a, c - a, c - a);
            if (glm::dot(ac_ortho, -a) > 0.0f)
            {
                // remove b
                simplex.erase(simplex.begin() + 1);
                axis = ac_ortho;
                return false;
            }

            // origin inside triangle
            return true;
        }
        return false;
    }
}
