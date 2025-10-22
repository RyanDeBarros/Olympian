#include "KinematicPhysicsComponent.h"

#include "core/util/Time.h"
#include "core/math/Geometry.h"

namespace oly::physics
{
	void KinematicPhysicsProperties::set_mass(float m)
	{
		_mass = m;
		_mass_inverse = 1.0f / _mass;
		if (use_moi_multiplier)
		{
			_moi = m * _moi_multiplier;
			_moi_inverse = 1.0f / _moi;
		}
	}

	void KinematicPhysicsProperties::set_moi(float m)
	{
		_moi = m;
		_moi_inverse = 1.0f / _moi;
		if (use_moi_multiplier)
		{
			_mass = _moi / _moi_multiplier;
			_mass_inverse = 1.0f / _mass;
		}
	}

	void KinematicPhysicsProperties::set_moi_multiplier(float mult)
	{
		use_moi_multiplier = true;
		_moi_multiplier = mult;
		_moi = _moi_multiplier * _mass;
		_moi_inverse = 1.0f / _moi;
	}

	glm::vec2 KinematicPhysicsProperties::dv_psi() const
	{
		glm::vec2 dv = {};

		if (dirty_linear_applied_accelerations)
		{
			dirty_linear_applied_accelerations = false;
			_net_linear_applied_acceleration = {};
			for (AppliedAcceleration accel : applied_accelerations)
				_net_linear_applied_acceleration += accel.acceleration;
		}
		dv += (net_linear_acceleration + _net_linear_applied_acceleration) * TIME.delta();

		if (dirty_linear_applied_forces)
		{
			dirty_linear_applied_forces = false;
			_net_linear_applied_force = {};
			for (AppliedForce force : applied_forces)
				_net_linear_applied_force += force.force;
		}
		dv += (net_force + _net_linear_applied_force) * TIME.delta() * _mass_inverse.get();

		glm::vec2 accumul_net_lin_impulse = net_linear_impulse;
		for (AppliedImpulse impulse : applied_impulses)
			accumul_net_lin_impulse += impulse.impulse;
		dv += accumul_net_lin_impulse * _mass_inverse.get();

		return dv;
	}

	float KinematicPhysicsProperties::dw_psi() const
	{
		float dw = 0.0f;

		dw += net_angular_acceleration * TIME.delta();
		dw += net_torque * TIME.delta() * _moi_inverse;
		dw += net_angular_impulse * _moi_inverse;

		if (dirty_angular_applied_accelerations)
		{
			dirty_angular_applied_accelerations = false;
			_net_angular_applied_acceleration = 0.0f;
			for (AppliedAcceleration accel : applied_accelerations)
				_net_angular_applied_acceleration += math::cross(accel.contact - center_of_mass, accel.acceleration);
		}
		dw += _net_angular_applied_acceleration * TIME.delta() * _mass * _moi_inverse;

		if (dirty_angular_applied_forces)
		{
			dirty_angular_applied_forces = false;
			_net_angular_applied_force = 0.0f;
			for (AppliedForce force : applied_forces)
				_net_angular_applied_force += math::cross(force.contact - center_of_mass, force.force);
		}
		dw += _net_angular_applied_force * TIME.delta() * _moi_inverse;

		float net_applied_angular_impulse = 0.0f;
		for (AppliedImpulse impulse : applied_impulses)
			net_applied_angular_impulse += math::cross(impulse.contact - center_of_mass, impulse.impulse);
		dw += net_applied_angular_impulse * _moi_inverse;

		return dw;
	}

	// DOC update latex on resolution bias removal and collision damping variables.

	void KinematicPhysicsComponent::post_tick() const
	{
		if (!collisions.empty())
		{
			// 1. update velocity from non-collision stimuli
			glm::vec2 new_linear_velocity = pre_state.linear_velocity + properties.dv_psi();
			float new_angular_velocity = pre_state.angular_velocity + properties.dw_psi();

			// 2. compute collision response
			compute_collision_response(new_linear_velocity, new_angular_velocity);
			compute_collision_mtv_idxs();

			// 3. update linear motion
			update_colliding_linear_motion(new_linear_velocity);

			// 4. update angular motion
			update_colliding_angular_motion(new_angular_velocity);
		}
		else
		{
			// 1. update velocity from stimuli
			post_state.linear_velocity += properties.dv_psi();
			post_state.angular_velocity += properties.dw_psi();

			// 2. apply drag
			if (submaterial->linear_drag > 0.0f)
				post_state.linear_velocity *= glm::exp(-submaterial->linear_drag * TIME.delta());
			if (submaterial->angular_drag > 0.0f)
				post_state.angular_velocity *= glm::exp(-submaterial->angular_drag * TIME.delta());

			// 3. update position
			post_state.position += post_state.linear_velocity * TIME.delta();

			// 4. update rotation
			post_state.rotation += post_state.angular_velocity * TIME.delta();
		}

		// 5. normalize rotation

		post_state.rotation = unsigned_fmod(post_state.rotation, glm::two_pi<float>());

		// 6. snap motion

		snap_motion();

		// 7. clean up

		collision_linear_impulse = {};
		collision_angular_impulse = 0.0f;
		properties.applied_impulses.clear();
		properties.net_linear_impulse = {};
		properties.net_angular_impulse = 0.0f;
		was_colliding = !collisions.empty();
		collisions.clear();
	}

	void KinematicPhysicsComponent::update_colliding_linear_motion(glm::vec2 new_velocity) const
	{
		// determine teleportation and update angular collision impulse

		const CollisionResponse& primary_collision = collisions[primary_collision_mtv_idx];
		glm::vec2 teleport = primary_collision.mtv * teleport_factor(*primary_collision.dynamics);
		collision_angular_impulse += math::cross(primary_collision.contact - properties.center_of_mass, teleport * TIME.inverse_delta() * properties.mass() + collision_linear_impulse);

		if (found_secondary_collision_mtv_idx)
		{
			const CollisionResponse& secondary_collision = collisions[secondary_collision_mtv_idx];
			glm::vec2 secondary_teleport = primary_collision.normal.perp_project(secondary_collision.mtv) * teleport_factor(*secondary_collision.dynamics);
			collision_angular_impulse += math::cross(secondary_collision.contact - properties.center_of_mass, secondary_teleport * TIME.inverse_delta() * properties.mass() + collision_linear_impulse);
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
		if (submaterial->linear_drag > 0.0f)
			post_state.linear_velocity *= glm::exp(-submaterial->linear_drag * TIME.delta());
	}

	void KinematicPhysicsComponent::update_colliding_angular_motion(float new_velocity) const
	{
		// determine teleportation

		const CollisionResponse& primary_collision = collisions[primary_collision_mtv_idx];
		float teleport = math::cross(primary_collision.contact - properties.center_of_mass, primary_collision.mtv) * teleport_factor(*primary_collision.dynamics);
		if (found_secondary_collision_mtv_idx)
		{
			const CollisionResponse& secondary_collision = collisions[secondary_collision_mtv_idx];
			glm::vec2 mtv = primary_collision.normal.perp_project(secondary_collision.mtv);
			teleport += math::cross(secondary_collision.contact - properties.center_of_mass, mtv) * teleport_factor(*secondary_collision.dynamics);
		}

		// dampen teleportation

		teleport *= properties.mass() * properties.moi_inverse();
		if (glm::abs(teleport) >= submaterial->angular_collision_damping.teleportation_jitter_threshold)
			teleport = glm::sign(teleport) * (1.0f - submaterial->angular_collision_damping.teleportation.inner())
			* glm::log(glm::abs(teleport) * submaterial->angular_collision_damping.teleportation_inverse_drag + 1.0f);
		else
			teleport = 0.0f;

		// dampen bounce

		float bounce = collision_angular_impulse * properties.moi_inverse();
		if (glm::abs(bounce) >= submaterial->angular_collision_damping.restitution_jitter_threshold)
			bounce = glm::sign(bounce) * (1.0f - submaterial->angular_collision_damping.restitution.inner())
			* glm::log(glm::abs(bounce) * submaterial->angular_collision_damping.restitution_inverse_drag + 1.0f);
		else
			bounce = 0.0f;

		// update new velocity

		new_velocity += bounce + teleport * TIME.inverse_delta();

		// update rotation

		post_state.rotation += new_velocity * TIME.delta();

		// update velocity

		post_state.angular_velocity = new_velocity;
		if (submaterial->angular_drag > 0.0f)
			post_state.angular_velocity *= glm::exp(-submaterial->angular_drag * TIME.delta());
	}

	// TODO v6 test simultaneous collision with multiple objects with complex_teleportation = true/false.
	void KinematicPhysicsComponent::compute_collision_mtv_idxs() const
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

	void KinematicPhysicsComponent::compute_collision_response(const glm::vec2 new_linear_velocity, const float new_angular_velocity) const
	{
		collision_linear_impulse = {};
		collision_angular_impulse = 0.0f;
		for (const CollisionResponse& collision : collisions)
		{
			float eff_mass = effective_mass(collision);
			glm::vec2 j_r = restitution_impulse(collision, eff_mass);
			glm::vec2 j_f = friction_impulse(collision, eff_mass, new_linear_velocity, new_angular_velocity);
			glm::vec2 impulse = j_r + j_f;
			collision_linear_impulse += impulse;
			collision_angular_impulse += math::cross(collision.contact - properties.center_of_mass, impulse);
		}
	}

	float KinematicPhysicsComponent::eff_mass_denom_factor(glm::vec2 local_contact, UnitVector2D normal) const
	{
		float cross = math::cross(local_contact - properties.center_of_mass, normal);
		return properties.mass_inverse() + properties.moi_inverse() * cross * cross;
	}

	glm::vec2 KinematicPhysicsComponent::contact_velocity(glm::vec2 local_contact) const
	{
		glm::vec2 contact = local_contact - properties.center_of_mass;
		return pre_state.linear_velocity + pre_state.angular_velocity * glm::vec2{ -contact.y, contact.x };
	}

	glm::vec2 KinematicPhysicsComponent::restitution_impulse(const CollisionResponse& collision, float eff_mass) const
	{
		const float restitution = restitution_with(collision);
		if (near_zero(restitution))
			return {};

		return std::max(-eff_mass * restitution * collision.normal.dot(relative_contact_velocity(collision)), 0.0f) * (glm::vec2)collision.normal;
	}

	glm::vec2 KinematicPhysicsComponent::friction_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 new_linear_velocity, float new_angular_velocity) const
	{
		const float mu = friction_with(collision);
		if (col2d::near_zero(mu))
			return {};

		glm::vec2 contact = collision.contact - properties.center_of_mass;
		glm::vec2 new_relative_velocity = new_linear_velocity + new_angular_velocity * glm::vec2{ -contact.y, contact.x } - other_contact_velocity(collision);
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

	void KinematicPhysicsComponent::snap_motion() const
	{
		// angular snapping
		if (properties.angular_snapping.enable && (!collisions.empty() || !properties.angular_snapping.only_colliding))
		{
			const auto& angular_snapping = submaterial->angular_snapping;
			if (!angular_snapping.snaps.empty())
			{
				if (glm::abs(post_state.angular_velocity) <= angular_snapping.speed_threshold)
				{
					const float snap_to = algo::find_closest(angular_snapping.snaps, post_state.rotation,
						[](decltype(angular_snapping.snaps)::key_type a, decltype(angular_snapping.snaps)::key_type b) { return unsigned_fmod(b - a, glm::two_pi<float>()); });

					float snap_by = snap_to - post_state.rotation;
					// rotate by shortest angle
					if (snap_by > glm::pi<float>())
						snap_by -= glm::two_pi<float>();
					else if (snap_by < -glm::pi<float>())
						snap_by += glm::two_pi<float>();

					if (glm::abs(snap_by) <= angular_snapping.angle_threshold)
					{
						const float proportion = glm::abs(snap_by / angular_snapping.angle_threshold);
						post_state.rotation += snap_by * (1.0f + (angular_snapping.strength_offset - 1.0f) * glm::pow(proportion, angular_snapping.strength));
					}
				}
			}
		}

		// linear snapping
		static const auto linear_snapping = [](const LinearSnapping& snapping, State& state, glm::length_t dim) {
			if (glm::abs(state.linear_velocity[dim]) <= snapping.speed_threshold)
			{
				float snap_by = round((state.position[dim] - snapping.snap_offset) / snapping.snap_width) * snapping.snap_width + snapping.snap_offset - state.position[dim];
				if (glm::abs(snap_by) <= snapping.position_threshold)
				{
					const float proportion = glm::abs(snap_by / snapping.position_threshold);
					state.position[dim] += snap_by * (1.0f + (snapping.strength_offset - 1.0f) * glm::pow(proportion, snapping.strength));
				}
			}
			};

		// linear snapping (X)
		if (properties.linear_x_snapping.enable && (!collisions.empty() || !properties.linear_x_snapping.only_colliding))
			linear_snapping(submaterial->linear_x_snapping, post_state, 0);

		// linear snapping (Y)
		if (properties.linear_y_snapping.enable && (!collisions.empty() || !properties.linear_y_snapping.only_colliding))
			linear_snapping(submaterial->linear_y_snapping, post_state, 1);
	}
}
