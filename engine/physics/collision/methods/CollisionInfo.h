#pragma once

#include "core/base/UnitVector.h"
#include "physics/collision/Tolerance.h"

#include <vector>

namespace oly::col2d
{
    struct ContactManifold
    {
        glm::vec2 p1 = {};
        glm::vec2 p2 = {};
        bool single = true;

        static void clamp(
            UnitVector2D axis,
            const ContactManifold c1,
            const ContactManifold c2,
            glm::vec2& p1,
            glm::vec2& p2);
        
        glm::vec2 pt() const;
    };

    struct OverlapResult
    {
        bool overlap = false;

        OverlapResult(bool overlap = false);

        operator bool() const;
        bool operator!() const;

        OverlapResult& invert();
    };

    struct CollisionResult
    {
        bool overlap = false;
        float penetration_depth = 0.f;
        UnitVector2D unit_impulse;

        // minimum translation vector
        glm::vec2 mtv() const;
        CollisionResult& invert();
    };

    struct ContactResult
    {
        struct Contact
        {
            glm::vec2 position = { 0.0f, 0.0f };
            glm::vec2 impulse = { 0.0f, 0.0f };
        };

        bool overlap = false;
        Contact active_contact;
        Contact passive_contact;

        ContactResult& invert();
    };

    template<typename C1, typename C2>
    inline ContactResult standard_contact_result(const C1& c1, const C2& c2, UnitVector2D axis, float depth)
    {
        ContactResult contact{ .overlap = true };
        contact.active_contact.impulse = depth * (glm::vec2)axis;
        contact.passive_contact.impulse = -contact.active_contact.impulse;

        ContactManifold m1 = c1.deepest_manifold(-axis);
        ContactManifold m2 = c2.deepest_manifold(axis);
        glm::vec2 p1 = {}, p2 = {};
        ContactManifold::clamp(axis, m1, m2, p1, p2);

        contact.active_contact.position = p1;
        contact.passive_contact.position = p2;
        return contact;
    }

    template<typename C1, typename C2>
    inline ContactResult standard_contact_result(const C1& c1, const C2& c2, const CollisionResult& collision)
    {
        if (collision.overlap)
            return standard_contact_result(c1, c2, collision.unit_impulse, collision.penetration_depth);
        else
            return { .overlap = false };
    }

    struct Ray
    {
        glm::vec2 origin;
        UnitVector2D direction;
        float clip = 0.0f;
    };

    struct RaycastResult
    {
        enum class Hit
        {
            NoHit,
            EmbeddedOrigin,
            TrueHit
        };

        Hit hit;
        glm::vec2 contact;
        UnitVector2D normal;
    };
}
