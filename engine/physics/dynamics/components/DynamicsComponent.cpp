#include "DynamicsComponent.h"

namespace oly::physics
{
    FrictionType friction_type(
        UnitVector2D tangent,
        glm::vec2 relative_contact_velocity,
        glm::vec2 relative_linear_velocity,
        float angular_velocity_a,
        float angular_velocity_b,
        PFloat speed_threshold)
    {
        if (!near_zero(tangent.dot(relative_contact_velocity), speed_threshold))
            return FrictionType::Kinetic;
        else if (!near_zero(tangent.dot(relative_linear_velocity), speed_threshold))
            return FrictionType::Rolling;
        else if (glm::sign(angular_velocity_a) != glm::sign(angular_velocity_b))
            return FrictionType::Rolling;
        else
            return FrictionType::Static;
    }

    void DynamicsComponent::post_tick() const
    {
        was_colliding_ = !collisions_.empty();
        collisions_.clear();
    }

    std::optional<float> DynamicsComponent::teleport_mass() const
    {
        return std::nullopt;
    }

    float DynamicsComponent::eff_mass_denom_factor(glm::vec2 local_contact, UnitVector2D normal) const
    {
        return 0.0f;
    }

    glm::vec2 DynamicsComponent::contact_velocity(glm::vec2 local_contact) const
    {
        return pre_state_.linear_velocity;
    }

    State DynamicsComponent::get_state() const
    {
        return post_state_;
    }

    void DynamicsComponent::add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& dynamics) const
    {
        collisions_.emplace_back(mtv, contact, UnitVector2D(mtv), &dynamics);
    }

    bool DynamicsComponent::is_colliding() const
    {
        return was_colliding_;
    }

    void DynamicsComponent::pre_tick(const glm::mat3& global) const
    {
        post_state_.position = global[2];
        post_state_.rotation = UnitVector2D(global[0]).rotation();
        pre_state_ = post_state_;
    }

    float DynamicsComponent::effective_mass(const CollisionResponse& collision) const
    {
        const float o_denom = collision.dynamics->eff_mass_denom_factor(
            collision.contact + pre_state_.position - collision.dynamics->pre_state_.position,
            -collision.normal);

        const float denom = eff_mass_denom_factor(collision.contact, -collision.normal) + o_denom;
        return denom != 0.0f ? 1.0f / denom : 0.0f;
    }

    glm::vec2 DynamicsComponent::other_contact_velocity(const CollisionResponse& collision) const
    {
        return collision.dynamics->contact_velocity(
            collision.contact + pre_state_.position - collision.dynamics->pre_state_.position);
    }

    glm::vec2 DynamicsComponent::relative_contact_velocity(const CollisionResponse& collision) const
    {
        return contact_velocity(collision.contact) - other_contact_velocity(collision);
    }

    float DynamicsComponent::restitution_with(const CollisionResponse& collision) const
    {
        return material->restitution.restitution_with(collision.dynamics->material->restitution);
    }

    float DynamicsComponent::friction_with(const CollisionResponse& collision) const
    {
        const FrictionType friction_type = physics::friction_type(
            -collision.normal.get_quarter_turn(),
            relative_contact_velocity(collision),
            pre_state_.linear_velocity - collision.dynamics->pre_state_.linear_velocity,
            pre_state_.angular_velocity,
            collision.dynamics->pre_state_.angular_velocity,
            static_cast<float>(col2d::kLinearTolerance));

        return material->friction.friction_with(collision.dynamics->material->friction, friction_type);
    }

    float DynamicsComponent::teleport_factor(const DynamicsComponent& other) const
    {
        std::optional<float> m1 = teleport_mass();
        if (m1.has_value())
        {
            std::optional<float> m2 = other.teleport_mass();
            if (m2.has_value())
                return m2.value() / (m1.value() + m2.value());
            else
                return 1.0f;
        }
        else
            return 0.0f;
    }
}
