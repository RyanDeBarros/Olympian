#include "DynamicsComponent.h"

#include "core/util/Time.h"
#include "core/math/Geometry.h"
#include "physics/dynamics/bodies/RigidBody.h"

namespace oly::physics
{
	FrictionType friction_type(UnitVector2D tangent, glm::vec2 relative_contact_velocity, glm::vec2 relative_linear_velocity,
		float angular_velocity_A, float angular_velocity_B, PositiveFloat speed_threshold)
	{
		if (!near_zero(tangent.dot(relative_contact_velocity), speed_threshold))
			return FrictionType::KINETIC;
		else if (!near_zero(tangent.dot(relative_linear_velocity), speed_threshold))
			return FrictionType::ROLLING;
		else if (glm::sign(angular_velocity_A) != glm::sign(angular_velocity_B))
			return FrictionType::ROLLING;
		else
			return FrictionType::STATIC;
	}

	// TODO v4 Time dilation (slo-mo).

	void DynamicsComponent::add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& dynamics) const
	{
		collisions.emplace_back(mtv, contact, UnitVector2D(mtv), &dynamics);
	}

	void DynamicsComponent::pre_tick(const glm::mat3& global) const
	{
		post_state.position = global[2];
		post_state.rotation = UnitVector2D(global[0]).rotation();
		pre_state = post_state;
	}

	void DynamicsComponent::post_tick() const
	{
		was_colliding = !collisions.empty();
		collisions.clear();
	}

	float DynamicsComponent::teleport_factor(const DynamicsComponent& other) const
	{
		std::optional<float> m1 = teleport_mass();
		if (m1.has_value())
		{
			std::optional<float> m2 = other.teleport_mass();
			if (m2.has_value())
				return m2.value() / (m1.value() + m2.value());
			else
				return 1.0f;
		}
		else
			return 0.0f;
	}

	float DynamicsComponent::effective_mass(const CollisionResponse& collision) const
	{
		float denom = eff_mass_denom_factor(collision.contact, -collision.normal)
			+ collision.dynamics->eff_mass_denom_factor(collision.contact + pre_state.position - collision.dynamics->pre_state.position, -collision.normal);
		return denom != 0.0f ? 1.0f / denom : 0.0f;
	}

	glm::vec2 DynamicsComponent::other_contact_velocity(const CollisionResponse& collision) const
	{
		return collision.dynamics->contact_velocity(collision.contact + pre_state.position - collision.dynamics->pre_state.position);
	}

	glm::vec2 DynamicsComponent::relative_contact_velocity(const CollisionResponse& collision) const
	{
		return contact_velocity(collision.contact) - other_contact_velocity(collision);
	}

	float DynamicsComponent::restitution_with(const CollisionResponse& collision) const
	{
		return material->restitution.restitution_with(collision.dynamics->material->restitution);
	}

	float DynamicsComponent::friction_with(const CollisionResponse& collision) const
	{
		UnitVector2D tangent = -collision.normal.get_quarter_turn();
		glm::vec2 relative_linear_velocity = pre_state.linear_velocity - collision.dynamics->pre_state.linear_velocity;
		FrictionType friction_type = physics::friction_type(tangent, relative_contact_velocity(collision),
			relative_linear_velocity, pre_state.angular_velocity, collision.dynamics->pre_state.angular_velocity, (float)col2d::LINEAR_TOLERANCE);
		return material->friction.friction_with(collision.dynamics->material->friction, friction_type);
	}
}
