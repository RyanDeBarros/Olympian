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
		PositiveFloat _dynamic_friction = 0.3f; // TODO kinetic vs rolling
		PositiveFloat _sqrt_dynamic_friction = glm::sqrt(_dynamic_friction);

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

		float dynamic_friction() const { return _dynamic_friction; }
		float sqrt_dynamic_friction() const { return _sqrt_dynamic_friction; }
		void set_dynamic_friction(float mu) { _dynamic_friction.set(mu); _sqrt_dynamic_friction.set(glm::sqrt(_dynamic_friction)); }
	};

	struct CollisionResponse
	{
		glm::vec2 mtv;
		glm::vec2 contact;
		const DynamicsComponent& other;
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
