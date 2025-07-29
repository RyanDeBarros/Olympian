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
	};

	extern FrictionType friction_type(UnitVector2D tangent, glm::vec2 relative_contact_velocity, glm::vec2 relative_linear_velocity,
		float angular_velocity_A, float angular_velocity_B, PositiveFloat speed_threshold = (float)col2d::LINEAR_TOLERANCE);

	class DynamicsComponent;

	struct CollisionResponse
	{
		glm::vec2 mtv;
		glm::vec2 contact;
		UnitVector2D normal;
		const DynamicsComponent* dynamics;
	};

	class DynamicsComponent
	{
	protected:
		mutable std::vector<CollisionResponse> collisions;
		mutable bool was_colliding = false;

		mutable State pre_state;
		mutable State post_state;

	public:
		// TODO v3 only restitution and friction are needed. Make CollisionDamping, Blending, AngularSnapping, and LinearSnapping optional components in Material.
		// The optional-ness can be set in constructor - Create FixedOptional struct that is an std::optional but once set/not set, cannot be set/unset.
		// Then in StaticPhysics/LinearPhysics/KinematicPhysics, construct it with proper state.
		MaterialRef material = REF_DEFAULT;

		virtual ~DynamicsComponent() = default;

	protected:
		virtual std::optional<float> teleport_mass() const { return std::nullopt; }
		virtual float eff_mass_denom_factor(glm::vec2 local_contact, UnitVector2D normal) const { return 0.0f; }
		float effective_mass(const CollisionResponse& collision) const;

		virtual glm::vec2 contact_velocity(glm::vec2 local_contact) const { return pre_state.linear_velocity; }
		glm::vec2 other_contact_velocity(const CollisionResponse& collision) const;
		glm::vec2 relative_contact_velocity(const CollisionResponse& collision) const;
		float restitution_with(const CollisionResponse& collision) const;
		float friction_with(const CollisionResponse& collision) const;

	public:
		State get_state() const { return post_state; }

		void add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& other) const;
		bool is_colliding() const { return was_colliding; }

		void pre_tick(const glm::mat3& global) const;
		virtual void post_tick() const;

	protected:
		float teleport_factor(const DynamicsComponent& other) const;
	};
}
