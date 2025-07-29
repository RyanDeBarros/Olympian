#pragma once

#include "physics/dynamics/bodies/RigidBody.h"
#include "physics/dynamics/SubMaterialComponents.h"

namespace oly::physics
{
	class LinearPhysicsComponent;

	struct LinearPhysicsProperties
	{
	private:
		StrictlyPositiveFloat _mass = 1.0f;
		StrictlyPositiveFloat _mass_inverse = 1.0f;

	public:
		float mass() const { return _mass; }
		float mass_inverse() const { return _mass_inverse; }
		void set_mass(float m) { _mass = m; _mass_inverse = 1.0f / _mass; }

	private:
		std::vector<glm::vec2> applied_accelerations;
		std::vector<glm::vec2> applied_forces;

	public:
		mutable std::vector<glm::vec2> applied_impulses;

		glm::vec2 net_acceleration = {}; // does not include applied accelerations
		glm::vec2 net_force = {}; // does not include applied forces
		mutable glm::vec2 net_impulse = {}; // does not include applied impulses

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

	private:
		friend class LinearPhysicsComponent;
		glm::vec2 dv_psi() const;

		mutable glm::vec2 _net_applied_acceleration = {};
		mutable glm::vec2 _net_applied_force = {};
		mutable bool dirty_applied_accelerations = false;
		mutable bool dirty_applied_forces = false;

	public:
		const std::vector<glm::vec2>& get_applied_accelerations() const { return applied_accelerations; }
		std::vector<glm::vec2>& set_applied_accelerations() { dirty_applied_accelerations = true; return applied_accelerations; }
		const std::vector<glm::vec2>& get_applied_forces() const { return applied_forces; }
		std::vector<glm::vec2>& set_applied_forces() { dirty_applied_forces = true; return applied_forces; }
	};

	struct LinearSubMaterial
	{
		PositiveFloat drag = 0.0f;
		LinearCollisionDamping collision_damping;
		LinearSnapping x_snapping, y_snapping;
	};

	typedef SmartReference<LinearSubMaterial> LinearSubMaterialRef;

	class LinearPhysicsComponent : public DynamicsComponent
	{
		mutable size_t primary_collision_mtv_idx = 0;
		mutable size_t secondary_collision_mtv_idx = 0;
		mutable bool found_secondary_collision_mtv_idx = false;
		mutable glm::vec2 collision_linear_impulse = {};

	public:
		LinearSubMaterialRef submaterial = REF_DEFAULT;
		LinearPhysicsProperties properties;

		void post_tick() const override;

	protected:
		std::optional<float> teleport_mass() const override { return properties.mass(); }
		float eff_mass_denom_factor(glm::vec2 local_contact, UnitVector2D normal) const override;

	private:
		void update_colliding_linear_motion(glm::vec2 new_velocity) const;
		void compute_collision_mtv_idxs() const;

		void compute_collision_response(glm::vec2 new_velocity) const;
		glm::vec2 restitution_impulse(const CollisionResponse& collision, float eff_mass) const;
		glm::vec2 friction_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 new_linear_velocity) const;

		void snap_motion() const;
	};

	class LinearBody : public RigidBody
	{
		OLY_COLLISION_CONTROLLER_HEADER(LinearBody);

	private:
		LinearPhysicsComponent dynamics;

	public:
		LinearBody();
		LinearBody(const LinearBody&);
		LinearBody(LinearBody&&) noexcept;
		~LinearBody();

	protected:
		void physics_pre_tick() override;
		void physics_post_tick() override;

	public:
		const MaterialRef& material() const { return dynamics.material; }
		MaterialRef& material() { return dynamics.material; }
		const LinearSubMaterialRef& sub_material() const { return dynamics.submaterial; }
		LinearSubMaterialRef& sub_material() { return dynamics.submaterial; }
		const LinearPhysicsProperties& properties() const { return dynamics.properties; }
		LinearPhysicsProperties& properties() { return dynamics.properties; }

		State state() const override { return dynamics.get_state(); }
		bool is_colliding() const override { return dynamics.is_colliding(); }

	protected:
		void bind(const col2d::Collider& collider) const override;
		void unbind(const col2d::Collider& collider) const override;

		const DynamicsComponent& get_dynamics() const override { return dynamics; }

	private:
		void handle_collides(const col2d::CollisionEventData& data) const;
	};

	typedef SmartReference<LinearBody> LinearBodyRef;
}
