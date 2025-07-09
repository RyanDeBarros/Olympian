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

		glm::vec2 linear_velocity_at(glm::vec2 contact) const;

		enum class FrictionType
		{
			STATIC,
			KINEMATIC
			// TODO ROLLING
		};

		FrictionType friction_type(glm::vec2 contact, UnitVector2D tangent, const State& other, PositiveFloat speed_threshold = (float)col2d::LINEAR_TOLERANCE) const;
	};

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
		PositiveFloat _mass;
		PositiveFloat _mass_inverse;
		// moment of inertia
		PositiveFloat _moi;
		PositiveFloat _moi_inverse;

	public:
		float mass() const { return _mass; }
		float mass_inverse() const { return _mass_inverse; }
		void set_mass(float m) { _mass.set(m); _mass_inverse.set(1.0f / m); }

		float moi() const { return _moi; }
		float moi_inverse() const { return _moi_inverse; }
		void set_moi(float m) { _moi.set(m); _moi_inverse.set(1.0f / m); }

		std::vector<AppliedAcceleration> applied_accelerations; // TODO should this be cached in net linear and in net angular?
		std::vector<AppliedForce> applied_forces; // TODO should this be cached in net linear and in net angular?
		std::vector<AppliedImpulse> applied_impulses; // TODO should this be cached in net linear and in net angular?
		glm::vec2 net_linear_acceleration = {}; // does not include applied accelerations
		glm::vec2 net_force = {}; // does not include applied forces
		glm::vec2 net_linear_impulse = {}; // does not include applied impulses
		float net_angular_acceleration = 0.0f;
		float net_torque = 0.0f;
		float net_angular_impulse = 0.0f;

	private:
		friend class DynamicsComponent;
		glm::vec2 dv_psi() const;
		float dw_psi() const;
	};

	// TODO helper functions for MOI

	struct Material
	{
	private:
		PositiveFloat _static_friction = 0.5f;
		PositiveFloat _sqrt_static_friction = glm::sqrt(_static_friction);
		PositiveFloat _kinematic_friction = 0.3f; // TODO kinetic vs rolling
		PositiveFloat _sqrt_kinematic_friction = glm::sqrt(_kinematic_friction);

	public:
		BoundedFloat<0.0f, 1.0f> restitution = 0.2f;
		PositiveFloat linear_drag = 0.0f;
		PositiveFloat angular_drag = 0.0f;
		BoundedFloat<0.0f, 1.0f> resolution_bias = 1.0f;
		std::optional<PositiveFloat> mass_density = std::nullopt; // TODO mass_density * area = mass
		std::optional<PositiveFloat> moi_density = std::nullopt; // TODO moi_density * area = moi

		float static_friction() const { return _static_friction; }
		float sqrt_static_friction() const { return _sqrt_static_friction; }
		void set_static_friction(float mu) { _static_friction.set(mu); _sqrt_static_friction.set(glm::sqrt(_static_friction)); }

		float kinematic_friction() const { return _kinematic_friction; }
		float sqrt_kinematic_friction() const { return _sqrt_kinematic_friction; }
		void set_kinematic_friction(float mu) { _kinematic_friction.set(mu); _sqrt_kinematic_friction.set(glm::sqrt(_kinematic_friction)); }

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
		mutable std::vector<CollisionResponse> static_collisions;
		mutable std::vector<CollisionResponse> kinematic_collisions;

	public:
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

	private:
		void compute_collision_response(glm::vec2& linear_impulse, float& angular_impulse) const;
	};
}
