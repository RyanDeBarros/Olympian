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
			KINEMATIC,
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
		ACTIVE,
	};

	extern void set_restitution_blend_op(FactorBlendOp op);
	extern void set_friction_blend_op(FactorBlendOp op);

	// LATER density
	struct Material
	{
	private:
		PositiveFloat _static_friction = 0.5f;
		PositiveFloat _sqrt_static_friction = glm::sqrt(_static_friction);
		PositiveFloat _kinematic_friction = 0.3f;
		PositiveFloat _sqrt_kinematic_friction = glm::sqrt(_kinematic_friction);
		PositiveFloat _rolling_friction = 0.4f;
		PositiveFloat _sqrt_rolling_friction = glm::sqrt(_rolling_friction);
		BoundedFloat<0.0f, 1.0f> _restitution = 0.2f;
		BoundedFloat<0.0f, 1.0f> _sqrt_restitution = glm::sqrt(_restitution);

	public:
		PositiveFloat linear_drag = 0.0f;
		PositiveFloat angular_drag = 0.0f;
		// Resolution bias controls collision response:
		//     -   0: No clipping, but less accurate bouncing.
		//     - 0.5: Accurate bouncing + low chance of clipping.
		//     -   1: Very accurate bouncing, but potential clipping.
		BoundedFloat<0.0f, 1.0f> resolution_bias = 0.5f;

		float static_friction() const { return _static_friction; }
		float sqrt_static_friction() const { return _sqrt_static_friction; }
		void set_static_friction(float mu) { _static_friction.set(mu); _sqrt_static_friction.set(glm::sqrt(_static_friction)); }

		float kinematic_friction() const { return _kinematic_friction; }
		float sqrt_kinematic_friction() const { return _sqrt_kinematic_friction; }
		void set_kinematic_friction(float mu) { _kinematic_friction.set(mu); _sqrt_kinematic_friction.set(glm::sqrt(_kinematic_friction)); }
		
		float rolling_friction() const { return _rolling_friction; }
		float sqrt_rolling_friction() const { return _sqrt_rolling_friction; }
		void set_rolling_friction(float mu) { _rolling_friction.set(mu); _sqrt_rolling_friction.set(glm::sqrt(_rolling_friction)); }

		float restitution() const { return _restitution; }
		float sqrt_restitution() const { return _sqrt_restitution; }
		void set_restitution(float e) { _restitution.set(e); _sqrt_restitution.set(glm::sqrt(_restitution)); }

	private:
		friend class DynamicsComponent;
		float restitution_with(const Material& mat) const;
		float friction_with(glm::vec2 contact, UnitVector2D tangent, const State& state, const Material& mat, const State& other_state, PositiveFloat speed_threshold = (float)col2d::LINEAR_TOLERANCE) const;
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
		// TODO possibly only need one vector
		mutable std::vector<CollisionResponse> static_collisions;
		mutable std::vector<CollisionResponse> kinematic_collisions;

	public:
		// TODO use handle to Material registry.
		Material material;
		Properties properties;

		enum class Flag
		{
			STATIC,
			KINEMATIC
		} flag = Flag::STATIC;

		void add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& other) const;

		void on_tick() const;
		State get_state() const { return state; }

		bool is_colliding() const { return !static_collisions.empty() || !kinematic_collisions.empty(); }

	private:
		void compute_collision_response(glm::vec2& linear_impulse, float& angular_impulse, glm::vec2 new_linear_velocity, float new_angular_velocity) const;
	};

	extern float moment_of_inertia(col2d::ElementParam e, float mass, bool relative_to_cm = false);
}
