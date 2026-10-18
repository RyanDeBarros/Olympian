#include "KinematicPhysicsComponent.h"

#include "core/math/Geometry.h"

#include "core/util/Time.h"

namespace oly::physics
{
    float KinematicPhysicsProperties::mass() const
    {
        return mass_;
    }

    float KinematicPhysicsProperties::mass_inverse() const
    {
        return mass_inverse_;
    }

    void KinematicPhysicsProperties::set_mass(float m)
    {
        mass_ = m;
        mass_inverse_ = 1.0f / mass_;
        if (use_moi_multiplier)
        {
            moi_ = m * moi_multiplier_;
            moi_inverse_ = 1.0f / moi_;
        }
    }

    float KinematicPhysicsProperties::moi() const
    {
        return moi_;
    }

    float KinematicPhysicsProperties::moi_inverse() const
    {
        return moi_inverse_;
    }

    void KinematicPhysicsProperties::set_moi(float m)
    {
        moi_ = m;
        moi_inverse_ = 1.0f / moi_;
        if (use_moi_multiplier)
        {
            mass_ = moi_ / moi_multiplier_;
            mass_inverse_ = 1.0f / mass_;
        }
    }

    float KinematicPhysicsProperties::moi_multiplier() const
    {
        return use_moi_multiplier ? static_cast<float>(moi_multiplier_) : moi_ * mass_inverse_;
    }

    void KinematicPhysicsProperties::set_moi_multiplier(float mult)
    {
        use_moi_multiplier = true;
        moi_multiplier_ = mult;
        moi_ = moi_multiplier_ * mass_;
        moi_inverse_ = 1.0f / moi_;
    }

    const std::vector<AppliedAcceleration>& KinematicPhysicsProperties::get_applied_accelerations() const
    {
        return applied_accelerations_;
    }

    std::vector<AppliedAcceleration>& KinematicPhysicsProperties::set_applied_accelerations()
    {
        dirty_linear_applied_accelerations_ = true;
        dirty_angular_applied_accelerations_ = true;
        return applied_accelerations_;
    }

    const std::vector<AppliedForce>& KinematicPhysicsProperties::get_applied_forces() const
    {
        return applied_forces_;
    }

    std::vector<AppliedForce>& KinematicPhysicsProperties::set_applied_forces()
    {
        dirty_linear_applied_forces_ = true;
        dirty_angular_applied_forces_ = true;
        return applied_forces_;
    }

    glm::vec2 KinematicPhysicsProperties::dv_psi() const
    {
        glm::vec2 dv = {};

        if (dirty_linear_applied_accelerations_)
        {
            dirty_linear_applied_accelerations_ = false;
            net_linear_applied_acceleration_ = {};
            for (AppliedAcceleration accel : applied_accelerations_)
                net_linear_applied_acceleration_ += accel.acceleration;
        }
        dv += (net_linear_acceleration + net_linear_applied_acceleration_) * kTime.delta();

        if (dirty_linear_applied_forces_)
        {
            dirty_linear_applied_forces_ = false;
            net_linear_applied_force_ = {};
            for (AppliedForce force : applied_forces_)
                net_linear_applied_force_ += force.force;
        }
        dv += (net_force + net_linear_applied_force_) * kTime.delta() * mass_inverse_.get();

        glm::vec2 accumul_net_lin_impulse = net_linear_impulse;
        for (AppliedImpulse impulse : applied_impulses)
            accumul_net_lin_impulse += impulse.impulse;
        dv += accumul_net_lin_impulse * mass_inverse_.get();

        return dv;
    }

    float KinematicPhysicsProperties::dw_psi() const
    {
        float dw = 0.0f;

        dw += net_angular_acceleration * kTime.delta();
        dw += net_torque * kTime.delta() * moi_inverse_;
        dw += net_angular_impulse * moi_inverse_;

        if (dirty_angular_applied_accelerations_)
        {
            dirty_angular_applied_accelerations_ = false;
            net_angular_applied_acceleration_ = 0.0f;
            for (AppliedAcceleration accel : applied_accelerations_)
                net_angular_applied_acceleration_ += math::cross(accel.contact - center_of_mass, accel.acceleration);
        }
        dw += net_angular_applied_acceleration_ * kTime.delta() * mass_ * moi_inverse_;

        if (dirty_angular_applied_forces_)
        {
            dirty_angular_applied_forces_ = false;
            net_angular_applied_force_ = 0.0f;
            for (AppliedForce force : applied_forces_)
                net_angular_applied_force_ += math::cross(force.contact - center_of_mass, force.force);
        }
        dw += net_angular_applied_force_ * kTime.delta() * moi_inverse_;

        float net_applied_angular_impulse = 0.0f;
        for (AppliedImpulse impulse : applied_impulses)
            net_applied_angular_impulse += math::cross(impulse.contact - center_of_mass, impulse.impulse);
        dw += net_applied_angular_impulse * moi_inverse_;

        return dw;
    }

    // DOC update latex on resolution bias removal and collision damping variables.

    void KinematicPhysicsComponent::post_tick() const
    {
        if (!collisions_.empty())
        {
            // 1. update velocity from non-collision stimuli
            glm::vec2 new_linear_velocity = pre_state_.linear_velocity + properties.dv_psi();
            float new_angular_velocity = pre_state_.angular_velocity + properties.dw_psi();

            // 2. compute collision response
            compute_collision_response(new_linear_velocity, new_angular_velocity);
            compute_collision_mtv_idxs();

            // 3. update linear motion
            update_colliding_linear_motion(new_linear_velocity);

            // 4. update angular motion
            update_colliding_angular_motion(new_angular_velocity);
        }
        else
        {
            // 1. update velocity from stimuli
            post_state_.linear_velocity += properties.dv_psi();
            post_state_.angular_velocity += properties.dw_psi();

            // 2. apply drag
            if (submaterial->linear_drag > 0.0f)
                post_state_.linear_velocity *= glm::exp(-submaterial->linear_drag * kTime.delta());
            if (submaterial->angular_drag > 0.0f)
                post_state_.angular_velocity *= glm::exp(-submaterial->angular_drag * kTime.delta());

            // 3. update position
            post_state_.position += post_state_.linear_velocity * kTime.delta();

            // 4. update rotation
            post_state_.rotation += post_state_.angular_velocity * kTime.delta();
        }

        // 5. normalize rotation

        post_state_.rotation = unsigned_fmod(post_state_.rotation, glm::two_pi<float>());

        // 6. snap motion

        snap_motion();

        // 7. clean up

        collision_linear_impulse = {};
        collision_angular_impulse = 0.0f;
        properties.applied_impulses.clear();
        properties.net_linear_impulse = {};
        properties.net_angular_impulse = 0.0f;
        was_colliding_ = !collisions_.empty();
        collisions_.clear();
    }

    std::optional<float> KinematicPhysicsComponent::teleport_mass() const
    {
        return properties.mass();
    }

    float KinematicPhysicsComponent::eff_mass_denom_factor(glm::vec2 local_contact, UnitVector2D normal) const
    {
        float cross = math::cross(local_contact - properties.center_of_mass, normal);
        return properties.mass_inverse() + properties.moi_inverse() * cross * cross;
    }

    glm::vec2 KinematicPhysicsComponent::contact_velocity(glm::vec2 local_contact) const
    {
        glm::vec2 contact = local_contact - properties.center_of_mass;
        return pre_state_.linear_velocity + pre_state_.angular_velocity * glm::vec2{ -contact.y, contact.x };
    }

    void KinematicPhysicsComponent::update_colliding_linear_motion(glm::vec2 new_velocity) const
    {
        // determine teleportation and update angular collision impulse

        const CollisionResponse& primary_collision = collisions_[primary_collision_mtv_idx];
        glm::vec2 teleport = primary_collision.mtv * teleport_factor(*primary_collision.dynamics);
        collision_angular_impulse += math::cross(
            primary_collision.contact - properties.center_of_mass,
            teleport * kTime.inverse_delta() * properties.mass() + collision_linear_impulse);

        if (found_secondary_collision_mtv_idx)
        {
            const CollisionResponse& secondary_collision = collisions_[secondary_collision_mtv_idx];
            glm::vec2 secondary_teleport = primary_collision.normal.perp_project(secondary_collision.mtv)
                * teleport_factor(*secondary_collision.dynamics);

            collision_angular_impulse += math::cross(
                secondary_collision.contact - properties.center_of_mass,
                secondary_teleport * kTime.inverse_delta() * properties.mass() + collision_linear_impulse);

            teleport += secondary_teleport;
        }

        // restrict velocity-based motion against teleportation

        UnitVector2D teleport_axis(teleport);
        float along_teleport_axis = std::max(teleport_axis.dot(new_velocity), 0.0f);
        UnitVector2D tangent_axis = teleport_axis.get_quarter_turn();
        float along_tangent_axis = tangent_axis.dot(new_velocity);
        new_velocity = along_tangent_axis * (glm::vec2)tangent_axis + along_teleport_axis * (glm::vec2)teleport_axis;

        if (glm::length(teleport) < submaterial->linear_collision_damping.teleportation_jitter_threshold)
            teleport = glm::vec2(0.0f);

        // update position

        post_state_.position += teleport + new_velocity * kTime.delta();

        // update velocity

        post_state_.linear_velocity = new_velocity + collision_linear_impulse * properties.mass_inverse();
        if (submaterial->linear_drag > 0.0f)
            post_state_.linear_velocity *= glm::exp(-submaterial->linear_drag * kTime.delta());
    }

    void KinematicPhysicsComponent::update_colliding_angular_motion(float new_velocity) const
    {
        // determine teleportation

        const CollisionResponse& primary_collision = collisions_[primary_collision_mtv_idx];
        float teleport = math::cross(
            primary_collision.contact - properties.center_of_mass,
            primary_collision.mtv) * teleport_factor(*primary_collision.dynamics);

        if (found_secondary_collision_mtv_idx)
        {
            const CollisionResponse& secondary_collision = collisions_[secondary_collision_mtv_idx];
            glm::vec2 mtv = primary_collision.normal.perp_project(secondary_collision.mtv);
            teleport += math::cross(secondary_collision.contact - properties.center_of_mass, mtv)
                * teleport_factor(*secondary_collision.dynamics);
        }

        // dampen teleportation

        teleport *= properties.mass() * properties.moi_inverse();
        if (glm::abs(teleport) >= submaterial->angular_collision_damping.teleportation_jitter_threshold)
            teleport = glm::sign(teleport) * (1.0f - submaterial->angular_collision_damping.teleportation.inner())
            * glm::log(glm::abs(teleport) * submaterial->angular_collision_damping.teleportation_inverse_drag + 1.0f);
        else
            teleport = 0.0f;

        // dampen bounce

        float bounce = collision_angular_impulse * properties.moi_inverse();
        if (glm::abs(bounce) >= submaterial->angular_collision_damping.restitution_jitter_threshold)
            bounce = glm::sign(bounce) * (1.0f - submaterial->angular_collision_damping.restitution.inner())
            * glm::log(glm::abs(bounce) * submaterial->angular_collision_damping.restitution_inverse_drag + 1.0f);
        else
            bounce = 0.0f;

        // update new velocity

        new_velocity += bounce + teleport * kTime.inverse_delta();

        // update rotation

        post_state_.rotation += new_velocity * kTime.delta();

        // update velocity

        post_state_.angular_velocity = new_velocity;
        if (submaterial->angular_drag > 0.0f)
            post_state_.angular_velocity *= glm::exp(-submaterial->angular_drag * kTime.delta());
    }

    void KinematicPhysicsComponent::compute_collision_mtv_idxs() const
    {
        primary_collision_mtv_idx = 0;
        float max_mag_sqrd = 0.0f;
        for (size_t i = 0; i < collisions_.size(); ++i)
        {
            float mag_sqrd = math::mag_sqrd(collisions_[i].mtv);
            if (mag_sqrd > max_mag_sqrd)
            {
                max_mag_sqrd = mag_sqrd;
                primary_collision_mtv_idx = i;
            }
        }

        secondary_collision_mtv_idx = 0;
        found_secondary_collision_mtv_idx = false;
        if (properties.complex_teleportation)
        {
            UnitVector2D primary_normal = collisions_[primary_collision_mtv_idx].normal.get_quarter_turn();
            float max_abs_proj = 0.0f;
            for (size_t i = 0; i < collisions_.size(); ++i)
            {
                if (i != primary_collision_mtv_idx)
                {
                    float abs_proj = glm::abs(primary_normal.dot(collisions_[i].mtv));
                    if (abs_proj > max_abs_proj)
                    {
                        max_abs_proj = abs_proj;
                        secondary_collision_mtv_idx = i;
                        found_secondary_collision_mtv_idx = true;
                    }
                }
            }
        }
    }

    void KinematicPhysicsComponent::compute_collision_response(
        const glm::vec2 new_linear_velocity,
        const float new_angular_velocity) const
    {
        collision_linear_impulse = {};
        collision_angular_impulse = 0.0f;
        for (const CollisionResponse& collision : collisions_)
        {
            float eff_mass = effective_mass(collision);
            glm::vec2 j_r = restitution_impulse(collision, eff_mass);
            glm::vec2 j_f = friction_impulse(collision, eff_mass, new_linear_velocity, new_angular_velocity);
            glm::vec2 impulse = j_r + j_f;
            collision_linear_impulse += impulse;
            collision_angular_impulse += math::cross(collision.contact - properties.center_of_mass, impulse);
        }
    }

    glm::vec2 KinematicPhysicsComponent::restitution_impulse(const CollisionResponse& collision, float eff_mass) const
    {
        const float restitution = restitution_with(collision);
        if (near_zero(restitution))
            return {};

        const float impulse = -eff_mass * restitution * collision.normal.dot(relative_contact_velocity(collision));
        return std::max(impulse, 0.0f) * (glm::vec2)collision.normal;
    }

    glm::vec2 KinematicPhysicsComponent::friction_impulse(
        const CollisionResponse& collision,
        float eff_mass,
        glm::vec2 new_linear_velocity,
        float new_angular_velocity) const
    {
        const float mu = friction_with(collision);
        if (col2d::near_zero(mu))
            return {};

        const glm::vec2 contact = collision.contact - properties.center_of_mass;
        const glm::vec2 new_relative_velocity = new_linear_velocity
            + new_angular_velocity * glm::vec2(-contact.y, contact.x)
            - other_contact_velocity(collision);
        
        const glm::vec2 new_tangent_velocity = collision.normal.perp_project(new_relative_velocity);
        const float new_tangent_velocity_sqrd = math::mag_sqrd(new_tangent_velocity);
        if (col2d::near_zero(new_tangent_velocity_sqrd))
            return {};

        const float normal_impulse = glm::length(collision.mtv)
            * teleport_factor(*collision.dynamics)
            * properties.mass()
            * kTime.inverse_delta();
        
        const float friction = std::min(mu * normal_impulse, eff_mass * glm::sqrt(new_tangent_velocity_sqrd));
        if (above_zero(friction))
            return -glm::normalize(new_tangent_velocity) * friction;
        else
            return {};
    }

    void KinematicPhysicsComponent::snap_motion() const
    {
        // angular snapping
        if (properties.angular_snapping.enable
            && (!collisions_.empty() || !properties.angular_snapping.only_colliding))
        {
            const auto& angular_snapping = submaterial->angular_snapping;
            if (!angular_snapping.snaps.empty())
            {
                if (glm::abs(post_state_.angular_velocity) <= angular_snapping.speed_threshold)
                {
                    const auto distance = [](BoundedRadians a, BoundedRadians b) {
                        return unsigned_fmod(b - a, glm::two_pi<float>());
                    };

                    const float snap_to = algo::find_closest(
                        angular_snapping.snaps,
                        post_state_.rotation,
                        distance);

                    float snap_by = snap_to - post_state_.rotation;
                    // rotate by shortest angle
                    if (snap_by > glm::pi<float>())
                        snap_by -= glm::two_pi<float>();
                    else if (snap_by < -glm::pi<float>())
                        snap_by += glm::two_pi<float>();

                    if (glm::abs(snap_by) <= angular_snapping.angle_threshold)
                    {
                        const float proportion = glm::abs(snap_by / angular_snapping.angle_threshold);
                        const float boost = (angular_snapping.strength_offset - 1.0f)
                            * glm::pow(proportion, angular_snapping.strength);
                        post_state_.rotation += snap_by * (1.0f + boost);
                    }
                }
            }
        }

        // linear snapping
        static const auto linear_snapping = [](const LinearSnapping& snapping, State& state, glm::length_t dim) {
            if (glm::abs(state.linear_velocity[dim]) <= snapping.speed_threshold)
            {
                const float rnd = round((state.position[dim] - snapping.snap_offset) / snapping.snap_width);
                const float snap_by = rnd * snapping.snap_width + snapping.snap_offset - state.position[dim];

                if (glm::abs(snap_by) <= snapping.position_threshold)
                {
                    const float proportion = glm::abs(snap_by / snapping.position_threshold);
                    const float boost = (snapping.strength_offset - 1.0f) * glm::pow(proportion, snapping.strength);
                    state.position[dim] += snap_by * (1.0f + boost);
                }
            }
        };

        // linear snapping (X)
        if (properties.linear_x_snapping.enable
            && (!collisions_.empty() || !properties.linear_x_snapping.only_colliding))
        {
            linear_snapping(submaterial->linear_x_snapping, post_state_, 0);
        }

        // linear snapping (Y)
        if (properties.linear_y_snapping.enable
            && (!collisions_.empty() || !properties.linear_y_snapping.only_colliding))
        {
            linear_snapping(submaterial->linear_y_snapping, post_state_, 1);
        }
    }
}
