#pragma once

#include "core/base/TransformerExposure.h"
#include "core/base/Parameters.h"
#include "physics/collision/scene/Collider.h"
#include "physics/dynamics/Material.h"

namespace oly::physics
{
	struct State
	{
		glm::vec2 position = {};
		float rotation = 0.0f;
		glm::vec2 linear_velocity = {};
		float angular_velocity = 0.0f;

		FrictionType friction_type(glm::vec2 contact, UnitVector2D tangent, const State& other, PositiveFloat speed_threshold = (float)col2d::LINEAR_TOLERANCE) const;
	};

	extern glm::vec2 linear_velocity_at(glm::vec2 linear_velocity, float angular_velocity, glm::vec2 contact);

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

	class DynamicsComponent;

	struct Properties
	{
	private:
		StrictlyPositiveFloat _mass = 1.0f;
		StrictlyPositiveFloat _mass_inverse = 1.0f;

		StrictlyPositiveFloat _moi = 1.0f;
		StrictlyPositiveFloat _moi_inverse = 1.0f;

		StrictlyPositiveFloat _moi_multiplier = 1.0f;

		std::vector<AppliedAcceleration> applied_accelerations;
		std::vector<AppliedForce> applied_forces;

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
			bool enable = true;
			bool only_colliding = false;
		} angular_snapping;

	private:
		friend class DynamicsComponent;
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

	struct CollisionResponse
	{
		glm::vec2 mtv;
		glm::vec2 contact;
		UnitVector2D normal;
		const DynamicsComponent* dynamics;
	};

	class DynamicsComponent
	{
		mutable State state;
		mutable std::vector<CollisionResponse> collisions;
		mutable bool was_colliding = false;

	public:
		enum class Flag
		{
			STATIC,
			KINEMATIC,
			LINEAR
		} flag = Flag::STATIC;

	private:
		mutable size_t primary_collision_mtv_idx = 0;
		mutable size_t secondary_collision_mtv_idx = 0;
		mutable bool found_secondary_collision_mtv_idx = false;
		mutable glm::vec2 collision_linear_impulse = {};
		mutable float collision_angular_impulse = 0.0f;

	public:
		MaterialRef material = REF_INIT;
		Properties properties;

		void add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& other) const;

		void on_tick() const;
		State get_state() const { return state; }
		void sync_state(const glm::mat3& global);

		bool is_colliding() const { return was_colliding; }

	private:
		void update_colliding_linear_motion(glm::vec2 new_velocity) const;
		void update_colliding_angular_motion(float new_velocity) const;
		void compute_collision_mtv_idxs() const;

		float teleport_factor(const DynamicsComponent& other) const;

		void compute_collision_response(glm::vec2 new_linear_velocity, float new_angular_velocity) const;
		glm::vec2 compute_other_contact_velocity(const CollisionResponse& collision) const;
		float effective_mass(const CollisionResponse& collision) const;
		glm::vec2 restitution_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 other_contact_velocity) const;
		glm::vec2 friction_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 other_contact_velocity, glm::vec2 new_linear_velocity, float new_angular_velocity) const;

		void snap_motion() const;
	};

	extern float moment_of_inertia(col2d::ElementParam e, float mass, bool relative_to_cm = false);
}
