#include "LinearPhysicsComponent.h"

#include "core/util/Time.h"
#include "core/math/Geometry.h"

namespace oly::physics
{
	glm::vec2 LinearPhysicsProperties::dv_psi() const
	{
		glm::vec2 dv = {};

		if (dirty_applied_accelerations)
		{
			dirty_applied_accelerations = false;
			_net_applied_acceleration = {};
			for (glm::vec2 accel : applied_accelerations)
				_net_applied_acceleration += accel;
		}
		dv += (net_acceleration + _net_applied_acceleration) * TIME.delta();

		if (dirty_applied_forces)
		{
			dirty_applied_forces = false;
			_net_applied_force = {};
			for (glm::vec2 force : applied_forces)
				_net_applied_force += force;
		}
		dv += (net_force + _net_applied_force) * TIME.delta() * _mass_inverse.get();

		glm::vec2 accumul_net_impulse = net_impulse;
		for (glm::vec2 impulse : applied_impulses)
			accumul_net_impulse += impulse;
		dv += accumul_net_impulse * _mass_inverse.get();

		return dv;
	}

	void LinearPhysicsComponent::post_tick() const
	{
		if (!collisions.empty())
		{
			// 1. update velocity from non-collision stimuli
			glm::vec2 new_velocity = pre_state.linear_velocity + properties.dv_psi();

			// 2. compute collision response
			compute_collision_response(new_velocity);
			compute_collision_mtv_idxs();

			// 3. update linear motion
			update_colliding_linear_motion(new_velocity);
		}
		else
		{
			// 1. update velocity from stimuli
			post_state.linear_velocity += properties.dv_psi();

			// 2. apply drag
			if (submaterial->drag > 0.0f)
				post_state.linear_velocity *= glm::exp(-submaterial->drag * TIME.delta());

			// 3. update position
			post_state.position += post_state.linear_velocity * TIME.delta();
		}

		// 6. snap motion

		snap_motion();

		// 7. clean up

		collision_linear_impulse = {};
		properties.applied_impulses.clear();
		properties.net_impulse = {};
		was_colliding = !collisions.empty();
		collisions.clear();
	}

	void LinearPhysicsComponent::update_colliding_linear_motion(glm::vec2 new_velocity) const
	{
		// determine teleportation

		const CollisionResponse& primary_collision = collisions[primary_collision_mtv_idx];
		glm::vec2 teleport = primary_collision.mtv * teleport_factor(*primary_collision.dynamics);

		if (found_secondary_collision_mtv_idx)
		{
			const CollisionResponse& secondary_collision = collisions[secondary_collision_mtv_idx];
			glm::vec2 secondary_teleport = primary_collision.normal.perp_project(secondary_collision.mtv) * teleport_factor(*secondary_collision.dynamics);
			teleport += secondary_teleport;
		}

		// restrict velocity-based motion against teleportation

		UnitVector2D teleport_axis(teleport);
		float along_teleport_axis = std::max(teleport_axis.dot(new_velocity), 0.0f);
		UnitVector2D tangent_axis = teleport_axis.get_quarter_turn();
		float along_tangent_axis = tangent_axis.dot(new_velocity);
		new_velocity = along_tangent_axis * (glm::vec2)tangent_axis + along_teleport_axis * (glm::vec2)teleport_axis;
		
		if (glm::length(teleport) < submaterial->linear_collision_damping.teleportation_jitter_threshold)
			teleport = glm::vec2(0.0f);

		// update position

		post_state.position += teleport + new_velocity * TIME.delta();

		// update velocity

		post_state.linear_velocity = new_velocity + collision_linear_impulse * properties.mass_inverse();
		if (submaterial->drag > 0.0f)
			post_state.linear_velocity *= glm::exp(-submaterial->drag * TIME.delta());
	}

	// TODO v5 test simultaneous collision with multiple objects with complex_teleportation = true/false.
	void LinearPhysicsComponent::compute_collision_mtv_idxs() const
	{
		primary_collision_mtv_idx = 0;
		float max_mag_sqrd = 0.0f;
		for (size_t i = 0; i < collisions.size(); ++i)
		{
			float mag_sqrd = math::mag_sqrd(collisions[i].mtv);
			if (mag_sqrd > max_mag_sqrd)
			{
				max_mag_sqrd = mag_sqrd;
				primary_collision_mtv_idx = i;
			}
		}

		secondary_collision_mtv_idx = 0;
		found_secondary_collision_mtv_idx = false;
		if (properties.complex_teleportation)
		{
			UnitVector2D primary_normal = collisions[primary_collision_mtv_idx].normal.get_quarter_turn();
			float max_abs_proj = 0.0f;
			for (size_t i = 0; i < collisions.size(); ++i)
			{
				if (i != primary_collision_mtv_idx)
				{
					float abs_proj = glm::abs(primary_normal.dot(collisions[i].mtv));
					if (abs_proj > max_abs_proj)
					{
						max_abs_proj = abs_proj;
						secondary_collision_mtv_idx = i;
						found_secondary_collision_mtv_idx = true;
					}
				}
			}
		}
	}

	void LinearPhysicsComponent::compute_collision_response(const glm::vec2 new_velocity) const
	{
		collision_linear_impulse = {};
		for (const CollisionResponse& collision : collisions)
		{
			float eff_mass = effective_mass(collision);
			glm::vec2 j_r = restitution_impulse(collision, eff_mass);
			glm::vec2 j_f = friction_impulse(collision, eff_mass, j_r, new_velocity);
			glm::vec2 impulse = j_r + j_f;
			collision_linear_impulse += impulse;
		}
	}

	float LinearPhysicsComponent::eff_mass_denom_factor(glm::vec2 local_contact, UnitVector2D normal) const
	{
		return properties.mass_inverse();
	}

	glm::vec2 LinearPhysicsComponent::restitution_impulse(const CollisionResponse& collision, float eff_mass) const
	{
		const float restitution = restitution_with(collision);
		if (near_zero(restitution))
			return {};

		return std::max(-eff_mass * restitution * collision.normal.dot(relative_contact_velocity(collision)), 0.0f) * (glm::vec2)collision.normal;
	}

	glm::vec2 LinearPhysicsComponent::friction_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 collision_impulse, glm::vec2 new_velocity) const
	{
		const float mu = friction_with(collision);
		if (col2d::near_zero(mu))
			return {};

		glm::vec2 new_relative_velocity = new_velocity - other_contact_velocity(collision);
		glm::vec2 new_tangent_velocity = collision.normal.perp_project(new_relative_velocity);
		float new_tangent_velocity_sqrd = math::mag_sqrd(new_tangent_velocity);
		if (col2d::near_zero(new_tangent_velocity_sqrd))
			return {};

		float normal_impulse = glm::length(collision.mtv) * teleport_factor(*collision.dynamics) * properties.mass() * TIME.inverse_delta();
		float friction = std::min(mu * normal_impulse, eff_mass * glm::sqrt(new_tangent_velocity_sqrd));
		if (above_zero(friction))
			return -glm::normalize(new_tangent_velocity) * friction;
		else
			return {};
	}

	void LinearPhysicsComponent::snap_motion() const
	{
		// linear snapping
		static const auto linear_snapping = [](const LinearSnapping& snapping, State& state, glm::length_t dim) {
			if (glm::abs(state.linear_velocity[dim]) <= snapping.speed_threshold)
			{
				float snap_by = int((state.position[dim] - snapping.snap_offset) / snapping.snap_width) * snapping.snap_width + snapping.snap_offset - state.position[dim];
				if (glm::abs(snap_by) <= snapping.position_threshold)
				{
					const float proportion = glm::abs(snap_by / snapping.position_threshold);
					state.position[dim] = snap_by * (1.0f + (snapping.strength_offset - 1.0f) * glm::pow(proportion, snapping.strength));
				}
			}
			};

		// linear snapping (X)
		if (properties.linear_x_snapping.enable && (!collisions.empty() || !properties.linear_x_snapping.only_colliding))
			linear_snapping(submaterial->x_snapping, post_state, 0);

		// linear snapping (Y)
		if (properties.linear_y_snapping.enable && (!collisions.empty() || !properties.linear_y_snapping.only_colliding))
			linear_snapping(submaterial->y_snapping, post_state, 1);
	}
}
