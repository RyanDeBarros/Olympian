#pragma once

#include "physics/dynamics/components/DynamicsComponent.h"

#include "physics/dynamics/components/materials/SubMaterialComponents.h"

namespace oly::physics
{
    class LinearPhysicsComponent;

    struct LinearPhysicsProperties
    {
        friend class LinearPhysicsComponent;

    private:
        StrictlyPositiveFloat mass_ = 1.0f;
        StrictlyPositiveFloat mass_inverse_ = 1.0f;

        std::vector<glm::vec2> applied_accelerations_;
        std::vector<glm::vec2> applied_forces_;

        mutable glm::vec2 net_applied_acceleration_ = {};
        mutable glm::vec2 net_applied_force_ = {};
        mutable bool dirty_applied_accelerations_ = false;
        mutable bool dirty_applied_forces_ = false;

    public:
        bool complex_teleportation = false;

        struct
        {
            bool enable = false;
            bool only_colliding = false;
        } linear_x_snapping;

        struct
        {
            bool enable = false;
            bool only_colliding = false;
        } linear_y_snapping;

        mutable std::vector<glm::vec2> applied_impulses;

        glm::vec2 net_acceleration = {}; // does not include applied accelerations
        glm::vec2 net_force = {}; // does not include applied forces
        mutable glm::vec2 net_impulse = {}; // does not include applied impulses

    public:
        float mass() const;
        float mass_inverse() const;
        void set_mass(float m);

        const std::vector<glm::vec2>& get_applied_accelerations() const;
        std::vector<glm::vec2>& set_applied_accelerations();
        const std::vector<glm::vec2>& get_applied_forces() const;
        std::vector<glm::vec2>& set_applied_forces();

    private:
        glm::vec2 dv_psi() const;
    };

    struct LinearSubMaterial
    {
        PositiveFloat drag = 0.0f;
        LinearCollisionDamping linear_collision_damping;
        LinearSnapping x_snapping;
        LinearSnapping y_snapping;
    };

    using LinearSubMaterialRef = SmartReference<LinearSubMaterial>;

    class LinearPhysicsComponent
        : public DynamicsComponent
    {
        mutable size_t primary_collision_mtv_idx_ = 0;
        mutable size_t secondary_collision_mtv_idx_ = 0;
        mutable bool found_secondary_collision_mtv_idx_ = false;
        mutable glm::vec2 collision_linear_impulse_ = {};

    public:
        LinearSubMaterialRef submaterial = REF_DEFAULT;
        LinearPhysicsProperties properties;

    public:
        void post_tick() const override;

    protected:
        std::optional<float> teleport_mass() const override;
        float eff_mass_denom_factor(glm::vec2 local_contact, UnitVector2D normal) const override;

    private:
        void update_colliding_linear_motion(glm::vec2 new_velocity) const;
        void compute_collision_mtv_idxs() const;

        void compute_collision_response(glm::vec2 new_velocity) const;
        glm::vec2 restitution_impulse(const CollisionResponse& collision, float eff_mass) const;
        glm::vec2 friction_impulse(
            const CollisionResponse& collision,
            float eff_mass,
            glm::vec2 collision_impulse,
            glm::vec2 new_linear_velocity) const;

        void snap_motion() const;
    };
}
