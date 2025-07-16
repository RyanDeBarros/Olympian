#pragma once

#include "core/base/TransformerExposure.h"
#include "core/base/SimpleMath.h"
#include "physics/collision/scene/Collider.h"

namespace oly::physics
{
	struct State
	{
		glm::vec2 position = {};
		float rotation = 0.0f;
		glm::vec2 linear_velocity = {};
		float angular_velocity = 0.0f;

		enum class FrictionType
		{
			STATIC,
			KINETIC,
			ROLLING
		};

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
		// moment of inertia
		StrictlyPositiveFloat _moi = 1.0f;
		StrictlyPositiveFloat _moi_inverse = 1.0f;

		std::vector<AppliedAcceleration> applied_accelerations;
		std::vector<AppliedForce> applied_forces;

	public:
		float mass() const { return _mass; }
		float mass_inverse() const { return _mass_inverse; }
		void set_mass(float m) { _mass.set(m); _mass_inverse.set(1.0f / m); }

		float moi() const { return _moi; }
		float moi_inverse() const { return _moi_inverse; }
		void set_moi(float m) { _moi.set(m); _moi_inverse.set(1.0f / m); }

		mutable std::vector<AppliedImpulse> applied_impulses;
		glm::vec2 net_linear_acceleration = {}; // does not include applied accelerations
		glm::vec2 net_force = {}; // does not include applied forces
		mutable glm::vec2 net_linear_impulse = {}; // does not include applied impulses
		float net_angular_acceleration = 0.0f;
		float net_torque = 0.0f;
		mutable float net_angular_impulse = 0.0f;

		glm::vec2 center_of_mass = {};

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

	enum class FactorBlendOp
	{
		MINIMUM,
		GEOMETRIC_MEAN,
		ARITHMETIC_MEAN,
		ACTIVE
	};

	// LATER density
	struct Material
	{
	private:
		PositiveFloat _static_friction = 0.5f;
		PositiveFloat _sqrt_static_friction = glm::sqrt(_static_friction);
		PositiveFloat _kinetic_friction = 0.3f;
		PositiveFloat _sqrt_kinetic_friction = glm::sqrt(_kinetic_friction);
		PositiveFloat _rolling_friction = 0.4f;
		PositiveFloat _sqrt_rolling_friction = glm::sqrt(_rolling_friction);
		BoundedFloat<0.0f, 1.0f> _restitution = 0.2f;
		BoundedFloat<0.0f, 1.0f> _sqrt_restitution = glm::sqrt(_restitution);

	public:
		PositiveFloat linear_drag = 0.0f;
		PositiveFloat angular_drag = 0.0f;

		// penetration damping controls how much penetration motion into another collider is clamped.
		// At 1.0 - complete clamping, which may cause slight jitteriness or alternating colliding states.
		// At 0.0 - no clamping, which may cause clipping and significant friction due to high normal force.
		BoundedFloat<0.0f, 1.0f> penetration_damping = 0.5f;

		struct
		{
			FactorBlendOp restitution = FactorBlendOp::MINIMUM;
			FactorBlendOp static_friction = FactorBlendOp::GEOMETRIC_MEAN;
			FactorBlendOp kinetic_friction = FactorBlendOp::GEOMETRIC_MEAN;
			FactorBlendOp rolling_friction = FactorBlendOp::GEOMETRIC_MEAN;
		} blending;

		float static_friction() const { return _static_friction; }
		float sqrt_static_friction() const { return _sqrt_static_friction; }
		void set_static_friction(float mu) { _static_friction.set(mu); _sqrt_static_friction.set(glm::sqrt(_static_friction)); }

		float kinetic_friction() const { return _kinetic_friction; }
		float sqrt_kinetic_friction() const { return _sqrt_kinetic_friction; }
		void set_kinetic_friction(float mu) { _kinetic_friction.set(mu); _sqrt_kinetic_friction.set(glm::sqrt(_kinetic_friction)); }
		
		float rolling_friction() const { return _rolling_friction; }
		float sqrt_rolling_friction() const { return _sqrt_rolling_friction; }
		void set_rolling_friction(float mu) { _rolling_friction.set(mu); _sqrt_rolling_friction.set(glm::sqrt(_rolling_friction)); }

		float restitution() const { return _restitution; }
		float sqrt_restitution() const { return _sqrt_restitution; }
		void set_restitution(float e) { _restitution.set(e); _sqrt_restitution.set(glm::sqrt(_restitution)); }

	private:
		friend class DynamicsComponent;
		float restitution_with(const Material& mat) const;
		float friction_with(const Material& mat, State::FrictionType friction_type) const;
	};

	struct CollisionResponse
	{
		glm::vec2 mtv;
		glm::vec2 contact;
		UnitVector2D normal;
		const DynamicsComponent* dynamics;

		glm::vec2 dx_teleport(float active_mass) const;
		float dtheta_teleport(glm::vec2 active_center_of_mass, float active_mass, float active_moi_inverse) const;
	};

	class DynamicsComponent
	{
		mutable State state;
		mutable std::vector<CollisionResponse> collisions;
		mutable bool was_colliding = false;

	public:
		// TODO use handle to Material registry.
		Material material;
		Properties properties;

		enum class Flag
		{
			STATIC,
			KINEMATIC,
			LINEAR
		} flag = Flag::STATIC;

		void add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& other) const;

		void on_tick() const;
		State get_state() const { return state; }
		void sync_state(const glm::mat3& global);

		bool is_colliding() const { return was_colliding; }

	private:
		void update_colliding_linear_motion(glm::vec2 new_velocity, glm::vec2 collision_impulse) const;
		void update_colliding_angular_motion(float new_velocity, float collision_impulse) const;

		void compute_collision_response(glm::vec2& linear_impulse, float& angular_impulse, glm::vec2 new_linear_velocity, float new_angular_velocity) const;
		glm::vec2 compute_other_contact_velocity(const CollisionResponse& collision) const;
		float effective_mass(const CollisionResponse& collision) const;
		glm::vec2 restitution_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 other_contact_velocity) const;
		glm::vec2 friction_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 other_contact_velocity, glm::vec2 new_linear_velocity, float new_angular_velocity) const;
	};

	extern float moment_of_inertia(col2d::ElementParam e, float mass, bool relative_to_cm = false);
}
