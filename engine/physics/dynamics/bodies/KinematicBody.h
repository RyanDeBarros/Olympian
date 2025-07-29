#pragma once

#include "physics/dynamics/bodies/RigidBody.h"
#include "physics/dynamics/bodies/DynamicsComponent.h"

namespace oly::physics
{
	struct AppliedAcceleration
	{
		glm::vec2 acceleration = {};
		glm::vec2 contact = {};
	};

	struct AppliedForce
	{
		glm::vec2 force = {};
		glm::vec2 contact = {};
	};

	struct AppliedImpulse
	{
		glm::vec2 impulse = {};
		glm::vec2 contact = {};
	};

	class KinematicPhysicsComponent;

	struct KinematicPhysicsProperties
	{
	private:
		StrictlyPositiveFloat _mass = 1.0f;
		StrictlyPositiveFloat _mass_inverse = 1.0f;

		StrictlyPositiveFloat _moi = 1.0f;
		StrictlyPositiveFloat _moi_inverse = 1.0f;

		StrictlyPositiveFloat _moi_multiplier = 1.0f;

	public:
		float mass() const { return _mass; }
		float mass_inverse() const { return _mass_inverse; }
		void set_mass(float m);

		float moi() const { return _moi; }
		float moi_inverse() const { return _moi_inverse; }
		void set_moi(float m);

		bool use_moi_multiplier = true;
		float moi_multiplier() const { return use_moi_multiplier ? (float)_moi_multiplier : _moi * _mass_inverse; }
		void set_moi_multiplier(float mult);

	private:
		std::vector<AppliedAcceleration> applied_accelerations;
		std::vector<AppliedForce> applied_forces;

	public:
		mutable std::vector<AppliedImpulse> applied_impulses;
		glm::vec2 net_linear_acceleration = {}; // does not include applied accelerations
		glm::vec2 net_force = {}; // does not include applied forces
		mutable glm::vec2 net_linear_impulse = {}; // does not include applied impulses
		float net_angular_acceleration = 0.0f;
		float net_torque = 0.0f;
		mutable float net_angular_impulse = 0.0f;

		glm::vec2 center_of_mass = {};

		bool complex_teleportation = false;

		struct
		{
			bool enable = false;
			bool only_colliding = false;
		} angular_snapping;

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
		friend class KinematicPhysicsComponent;
		glm::vec2 dv_psi() const;
		float dw_psi() const;

		mutable glm::vec2 _net_linear_applied_acceleration = {};
		mutable float _net_angular_applied_acceleration = 0.0f;
		mutable glm::vec2 _net_linear_applied_force = {};
		mutable float _net_angular_applied_force = 0.0f;
		mutable bool dirty_linear_applied_accelerations = false;
		mutable bool dirty_angular_applied_accelerations = false;
		mutable bool dirty_linear_applied_forces = false;
		mutable bool dirty_angular_applied_forces = false;

	public:
		const std::vector<AppliedAcceleration>& get_applied_accelerations() const { return applied_accelerations; }
		std::vector<AppliedAcceleration>& set_applied_accelerations() { dirty_linear_applied_accelerations = true; dirty_angular_applied_accelerations = true; return applied_accelerations; }
		const std::vector<AppliedForce>& get_applied_forces() const { return applied_forces; }
		std::vector<AppliedForce>& set_applied_forces() { dirty_linear_applied_forces = true; dirty_angular_applied_forces = true; return applied_forces; }
	};

	class KinematicPhysicsComponent : public DynamicsComponent
	{
		mutable size_t primary_collision_mtv_idx = 0;
		mutable size_t secondary_collision_mtv_idx = 0;
		mutable bool found_secondary_collision_mtv_idx = false;
		mutable glm::vec2 collision_linear_impulse = {};
		mutable float collision_angular_impulse = 0.0f;

	public:
		KinematicPhysicsProperties properties;

		void post_tick() const override;

	protected:
		std::optional<float> teleport_mass() const override { return properties.mass(); }
		float eff_mass_denom_factor(glm::vec2 local_contact, UnitVector2D normal) const override;
		virtual glm::vec2 contact_velocity(glm::vec2 local_contact) const override;

	private:
		void update_colliding_linear_motion(glm::vec2 new_velocity) const;
		void update_colliding_angular_motion(float new_velocity) const;
		void compute_collision_mtv_idxs() const;

		void compute_collision_response(glm::vec2 new_linear_velocity, float new_angular_velocity) const;
		glm::vec2 restitution_impulse(const CollisionResponse& collision, float eff_mass) const;
		glm::vec2 friction_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 new_linear_velocity, float new_angular_velocity) const;

		void snap_motion() const;
	};

	class KinematicBody : public RigidBody
	{
		OLY_COLLISION_CONTROLLER_HEADER(KinematicBody);

	private:
		KinematicPhysicsComponent dynamics;

	public:
		KinematicBody();
		KinematicBody(const KinematicBody&);
		KinematicBody(KinematicBody&&) noexcept;
		~KinematicBody();

	protected:
		void physics_pre_tick() override;
		void physics_post_tick() override;

	public:
		const MaterialRef& material() const { return dynamics.material; }
		MaterialRef& material() { return dynamics.material; }
		const KinematicPhysicsProperties& properties() const { return dynamics.properties; }
		KinematicPhysicsProperties& properties() { return dynamics.properties; }

		State state() const override { return dynamics.get_state(); }
		bool is_colliding() const override { return dynamics.is_colliding(); }

	protected:
		void bind(const col2d::Collider& collider) const override;
		void unbind(const col2d::Collider& collider) const override;

		const DynamicsComponent& get_dynamics() const override { return dynamics; }

	private:
		void handle_contacts(const col2d::ContactEventData& data) const;
	};

	typedef SmartReference<KinematicBody> KinematicBodyRef;
}
