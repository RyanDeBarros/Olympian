#include "DynamicsComponent.h"

#include "core/util/Time.h"
#include "core/math/Geometry.h"
#include "physics/dynamics/RigidBody.h"

namespace oly::physics
{
	glm::vec2 linear_velocity_at(glm::vec2 linear_velocity, float angular_velocity, glm::vec2 contact)
	{
		return linear_velocity + angular_velocity * glm::vec2{ -contact.y, contact.x };
	}

	FrictionType State::friction_type(glm::vec2 contact, UnitVector2D tangent, const State& other, PositiveFloat speed_threshold) const
	{
		bool sliding = !near_zero(tangent.dot(linear_velocity_at(linear_velocity, angular_velocity, contact)
			- linear_velocity_at(other.linear_velocity, other.angular_velocity, contact + position - other.position)), speed_threshold);
		if (sliding)
			return FrictionType::KINETIC;
		else
			return near_zero(tangent.dot(linear_velocity) - tangent.dot(other.linear_velocity), speed_threshold) ? FrictionType::STATIC : FrictionType::ROLLING;
	}

	void Properties::set_mass(float m)
	{
		_mass = m;
		_mass_inverse = 1.0f / _mass;
		if (use_moi_multiplier)
		{
			_moi = m * _moi_multiplier;
			_moi_inverse = 1.0f / _moi;
		}
	}

	void Properties::set_moi(float m)
	{
		_moi = m;
		_moi_inverse = 1.0f / _moi;
		if (use_moi_multiplier)
		{
			_mass = _moi / _moi_multiplier;
			_mass_inverse = 1.0f / _mass;
		}
	}

	void Properties::set_moi_multiplier(float mult)
	{
		use_moi_multiplier = true;
		_moi_multiplier = mult;
		_moi = _moi_multiplier * _mass;
		_moi_inverse = 1.0f / _moi;
	}

	glm::vec2 Properties::dv_psi() const
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

	float Properties::dw_psi() const
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

	void DynamicsComponent::add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& dynamics) const
	{
		collisions.emplace_back(mtv, contact, UnitVector2D(mtv), &dynamics);
	}

	// LATER pausing mechanism for RigidBody/TIME so that dt doesn't keep increasing as game is paused. + Time dilation (slo-mo).

	// DOC update latex on resolution bias removal and collision damping variables.

	void DynamicsComponent::on_tick() const
	{
		if (flag != Flag::STATIC)
		{
			if (!collisions.empty())
			{
				// 1. update velocity from non-collision stimuli
				glm::vec2 new_linear_velocity = state.linear_velocity + properties.dv_psi();
				float new_angular_velocity = flag == Flag::KINEMATIC ? state.angular_velocity + properties.dw_psi() : 0.0f;

				// 2. compute collision response
				glm::vec2 j_c = {};
				float h_c = 0.0f;
				compute_collision_response(j_c, h_c, new_linear_velocity, new_angular_velocity);
				compute_collision_mtv_idxs();

				// 3. update linear motion
				update_colliding_linear_motion(new_linear_velocity, j_c);

				// 4. update angular motion
				if (flag == Flag::KINEMATIC)
					update_colliding_angular_motion(new_angular_velocity, h_c);
			}
			else
			{
				// 1. update velocity from stimuli
				state.linear_velocity += properties.dv_psi();
				if (flag == Flag::KINEMATIC)
					state.angular_velocity += properties.dw_psi();

				// 2. apply drag
				if (material->linear_drag > 0.0f)
					state.linear_velocity *= glm::exp(-material->linear_drag * TIME.delta());
				if (flag == Flag::KINEMATIC)
					if (material->angular_drag > 0.0f)
						state.angular_velocity *= glm::exp(-material->angular_drag * TIME.delta());

				// 3. update position
				state.position += state.linear_velocity * TIME.delta();

				// 4. update rotation
				if (flag == Flag::KINEMATIC)
					state.rotation += state.angular_velocity * TIME.delta();
			}
		}

		// 7. clean up

		properties.applied_impulses.clear();
		properties.net_linear_impulse = {};
		properties.net_angular_impulse = 0.0f;
		was_colliding = !collisions.empty();
		collisions.clear();
	}

	void DynamicsComponent::sync_state(const glm::mat3& global)
	{
		state.position = global[2];
		state.rotation = UnitVector2D(global[0]).rotation();
	}

	void DynamicsComponent::update_colliding_linear_motion(glm::vec2 new_velocity, glm::vec2 collision_impulse) const
	{
		// collision position update

		const CollisionResponse& primary_collision = collisions[primary_collision_mtv_idx];
		glm::vec2 teleport = primary_collision.mtv * teleport_factor(*primary_collision.dynamics);
		if (found_secondary_collision_mtv_idx)
		{
			const CollisionResponse& secondary_collision = collisions[secondary_collision_mtv_idx];
			teleport += primary_collision.normal.normal_project(secondary_collision.mtv) * teleport_factor(*secondary_collision.dynamics);
		}

		// natural position update

		UnitVector2D teleport_axis(teleport);
		glm::vec2 dx_v = new_velocity * TIME.delta();
		float along_teleport_axis = teleport_axis.dot(dx_v);
		glm::vec2 perp_teleport_axis = dx_v - along_teleport_axis * (glm::vec2)teleport_axis;

		along_teleport_axis = glm::max(along_teleport_axis, -glm::length(teleport));
		if (along_teleport_axis < 0.0f)
			along_teleport_axis *= 1.0f - material->collision_damping.linear_penetration;
		
		dx_v = perp_teleport_axis + along_teleport_axis * (glm::vec2)teleport_axis;
		state.position += teleport + dx_v;

		// velocity update

		state.linear_velocity = dx_v * TIME.inverse_delta() + collision_impulse * properties.mass_inverse();
		if (material->linear_drag > 0.0f)
			state.linear_velocity *= glm::exp(-material->linear_drag * TIME.delta());
	}

	void DynamicsComponent::update_colliding_angular_motion(float new_velocity, float collision_impulse) const
	{
		// collision rotation update

		const CollisionResponse& primary_collision = collisions[primary_collision_mtv_idx];
		float teleport = math::cross(primary_collision.contact - properties.center_of_mass, primary_collision.mtv) * teleport_factor(*primary_collision.dynamics);
		if (found_secondary_collision_mtv_idx)
		{
			const CollisionResponse& secondary_collision = collisions[secondary_collision_mtv_idx];
			glm::vec2 mtv = primary_collision.normal.normal_project(secondary_collision.mtv);
			teleport += math::cross(secondary_collision.contact - properties.center_of_mass, mtv) * teleport_factor(*secondary_collision.dynamics);
		}

		teleport *= properties.mass() * properties.moi_inverse();
		if (glm::abs(teleport) > material->collision_damping.angular_jitter_threshold)
			teleport = glm::sign(teleport) * (1.0f - material->collision_damping.angular_teleportation.inner())
				* glm::log(glm::abs(teleport) * material->collision_damping.angular_teleport_inverse_drag + 1.0f);
		else
			teleport = 0.0f;

		// natural rotation udpate

		state.rotation += new_velocity * TIME.delta();

		// velocity update

		state.angular_velocity = new_velocity + teleport * TIME.inverse_delta() + collision_impulse * properties.moi_inverse();
		if (material->angular_drag > 0.0f)
			state.angular_velocity *= glm::exp(-material->angular_drag * TIME.delta());
	}

	// TODO v2 test simultaneous collision with multiple objects with complex_teleportation = true/false.
	void DynamicsComponent::compute_collision_mtv_idxs() const
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
		if (complex_teleportation)
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

	float DynamicsComponent::teleport_factor(const DynamicsComponent& other) const
	{
		return other.flag != DynamicsComponent::Flag::STATIC ? other.properties.mass() / (properties.mass() + other.properties.mass()) : 1.0f;
	}

	void DynamicsComponent::compute_collision_response(glm::vec2& linear_impulse, float& angular_impulse, const glm::vec2 new_linear_velocity, const float new_angular_velocity) const
	{
		linear_impulse = {};
		angular_impulse = 0.0f;
		for (const CollisionResponse& collision : collisions)
		{
			float eff_mass = effective_mass(collision);
			glm::vec2 other_contact_velocity = compute_other_contact_velocity(collision);
			glm::vec2 j_r = restitution_impulse(collision, eff_mass, other_contact_velocity);
			glm::vec2 j_f = friction_impulse(collision, eff_mass, other_contact_velocity, new_linear_velocity, new_angular_velocity);
			glm::vec2 impulse = j_r + j_f;
			linear_impulse += impulse;
			if (flag == Flag::KINEMATIC)
				angular_impulse += math::cross(collision.contact - properties.center_of_mass, impulse);
		}
	}

	glm::vec2 DynamicsComponent::compute_other_contact_velocity(const CollisionResponse& collision) const
	{
		glm::vec2 other_contact_velocity = {};
		if (collision.dynamics->flag != Flag::STATIC)
			other_contact_velocity = linear_velocity_at(collision.dynamics->state.linear_velocity, collision.dynamics->state.angular_velocity,
				state.position + collision.contact - properties.center_of_mass - collision.dynamics->state.position);
		return other_contact_velocity;
	}

	float DynamicsComponent::effective_mass(const CollisionResponse& collision) const
	{
		float effective_mass_denominator = properties.mass_inverse();
		if (flag != Flag::LINEAR)
		{
			const float cross1 = math::cross(collision.contact - properties.center_of_mass, collision.normal);
			effective_mass_denominator += properties.moi_inverse() * cross1 * cross1;
		}
		if (collision.dynamics->flag != Flag::STATIC)
		{
			effective_mass_denominator += collision.dynamics->properties.mass_inverse();
			if (collision.dynamics->flag != Flag::LINEAR)
			{
				const float cross2 = math::cross(state.position + collision.contact - properties.center_of_mass - collision.dynamics->state.position, collision.normal);
				effective_mass_denominator += collision.dynamics->properties.moi_inverse() * cross2 * cross2;
			}
		}
		return 1.0f / effective_mass_denominator;
	}

	glm::vec2 DynamicsComponent::restitution_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 other_contact_velocity) const
	{
		const float restitution = material->restitution_with(*collision.dynamics->material);
		if (near_zero(restitution))
			return {};

		glm::vec2 relative_velocity = linear_velocity_at(state.linear_velocity, state.angular_velocity, collision.contact - properties.center_of_mass) - other_contact_velocity;
		return std::max(-eff_mass * restitution * collision.normal.dot(relative_velocity), 0.0f) * (glm::vec2)collision.normal;
	}

	glm::vec2 DynamicsComponent::friction_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 other_contact_velocity, glm::vec2 new_linear_velocity, float new_angular_velocity) const
	{
		FrictionType friction_type = state.friction_type(collision.contact - properties.center_of_mass, collision.normal.get_quarter_turn(), collision.dynamics->state);
		float mu = material->friction_with(*collision.dynamics->material, friction_type);
		if (col2d::near_zero(mu))
			return {};

		glm::vec2 new_relative_velocity = linear_velocity_at(new_linear_velocity, new_angular_velocity, collision.contact - properties.center_of_mass) - other_contact_velocity;
		glm::vec2 new_tangent_velocity = collision.normal.normal_project(new_relative_velocity);
		float new_tangent_velocity_sqrd = math::mag_sqrd(new_tangent_velocity);
		if (col2d::near_zero(new_tangent_velocity_sqrd))
			return {};


		float normal_impulse = collision.normal.dot(collision.mtv * teleport_factor(*collision.dynamics) * properties.mass() * TIME.inverse_delta());
		float friction = std::min(mu * normal_impulse, eff_mass * glm::sqrt(new_tangent_velocity_sqrd));
		if (above_zero(friction))
			return -glm::normalize(new_tangent_velocity) * friction;
		else
			return {};
	}

	static float moment_of_inertia(const math::Polygon2D& p, float mass, glm::vec2 offset = {})
	{
		float sum = 0.0f;
		for (size_t i = 0; i < p.size(); ++i)
		{
			glm::vec2 curr = p[i] - offset;
			glm::vec2 next = p[(i + 1) % p.size()] - offset;
			sum += math::cross(curr, next) * (math::mag_sqrd(curr) + glm::dot(curr, next) + math::mag_sqrd(next));
		}
		return sum * mass / (6.0f * math::signed_area(p));
	}

	static float moment_of_inertia(const std::array<glm::vec2, 4>& p, float mass, glm::vec2 offset = {})
	{
		return moment_of_inertia(math::Polygon2D{ p[0], p[1], p[2], p[3] }, mass, offset);
	}

	float moment_of_inertia(col2d::ElementParam e, float mass, bool relative_to_cm)
	{
		return std::visit([mass, relative_to_cm](const auto& e) {
			if constexpr (visiting_class_is<decltype(*e), col2d::Circle>)
			{
				if (relative_to_cm)
				{
					if (col2d::internal::CircleGlobalAccess::has_no_global(*e))
						return 0.5f * mass * e->radius * e->radius;
					else
					{
						const glm::mat2 ltl = glm::transpose(col2d::internal::CircleGlobalAccess::get_global(*e)) * col2d::internal::CircleGlobalAccess::get_global(*e);
						return 0.25f * mass * e->radius * e->radius * (ltl[0][0] + ltl[1][1]);
					}
				}
				else
				{
					if (col2d::internal::CircleGlobalAccess::has_no_global(*e))
						return 0.5f * mass * e->radius * e->radius + mass * math::mag_sqrd(col2d::internal::CircleGlobalAccess::global_center(*e));
					else
					{
						const glm::mat2 ltl = glm::transpose(col2d::internal::CircleGlobalAccess::get_global(*e)) * col2d::internal::CircleGlobalAccess::get_global(*e);
						return 0.25f * mass * e->radius * e->radius * (ltl[0][0] + ltl[1][1]) + mass * math::mag_sqrd(col2d::internal::CircleGlobalAccess::global_center(*e));
					}
				}
			}
			else if (relative_to_cm)
			{
				if constexpr (visiting_class_is<decltype(*e), col2d::OBB>)
					return moment_of_inertia(e->points(), mass, e->center);
				else
					return moment_of_inertia(e->points(), mass, e->center());
			}
			else
				return moment_of_inertia(e->points(), mass);
			}, e);
	}
}
