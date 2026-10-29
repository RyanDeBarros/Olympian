#include "CollisionInfo.h"

namespace oly::col2d
{
    void ContactManifold::clamp(
        UnitVector2D axis,
        const ContactManifold c1,
        const ContactManifold c2,
        glm::vec2& p1,
        glm::vec2& p2)
    {
        UnitVector2D tangent = axis.get_quarter_turn();

        float v11 = tangent.dot(c1.p1);
        float v12 = c1.single ? v11 : tangent.dot(c1.p2);
        if (v11 > v12)
            std::swap(v11, v12);
        float v21 = tangent.dot(c2.p1);
        float v22 = c2.single ? v21 : tangent.dot(c2.p2);
        if (v21 > v22)
            std::swap(v21, v22);

        float w1, w2;
        if (v12 < v21)
        {
            w1 = v12;
            w2 = v21;
        }
        else if (v11 > v22)
        {
            w1 = v11;
            w2 = v22;
        }
        else
        {
            v11 = glm::clamp(v11, v21, v22);
            v12 = glm::clamp(v12, v21, v22);
            w1 = 0.5f * (v11 + v12);
            w2 = w1;
        }

        glm::vec2 along_axis = axis.dot(c1.p2) * (glm::vec2)axis;
        p1 = along_axis + w1 * (glm::vec2)tangent;
        p2 = along_axis + w2 * (glm::vec2)tangent;
    }

    glm::vec2 ContactManifold::pt() const
    {
        return single ? p1 : 0.5f * (p1 + p2);
    }

    OverlapResult::OverlapResult(bool overlap)
        : overlap(overlap)
    {
    }

    OverlapResult::operator bool() const
    {
        return overlap;
    }

    bool OverlapResult::operator!() const
    {
        return !overlap;
    }

    OverlapResult& OverlapResult::invert()
    {
        return *this;
    }

    glm::vec2 CollisionResult::mtv() const
    {
        return (glm::vec2)unit_impulse * penetration_depth;
    }

    CollisionResult& CollisionResult::invert()
    {
        unit_impulse = -unit_impulse;
        return *this;
    }

    ContactResult& ContactResult::invert()
    {
        std::swap(active_contact, passive_contact);
        return *this;
    }
}
