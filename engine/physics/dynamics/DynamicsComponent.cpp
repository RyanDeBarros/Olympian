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

	State::FrictionType State::friction_type(glm::vec2 contact, UnitVector2D tangent, const State& other, PositiveFloat speed_threshold) const
	{
		bool sliding = !near_zero(tangent.dot(linear_velocity_at(linear_velocity, angular_velocity, contact)
			- linear_velocity_at(other.linear_velocity, other.angular_velocity, contact + position - other.position)), speed_threshold);
		if (sliding)
			return FrictionType::KINETIC;
		else // TODO more sophisticated distinction between STATIC/ROLLING - check for locked together case.
			return near_zero(angular_velocity) && near_zero(other.angular_velocity) ? FrictionType::STATIC : FrictionType::ROLLING;
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

	float Material::restitution_with(const Material& mat) const
	{
		switch (blending.restitution)
		{
		case FactorBlendOp::MINIMUM:
			return std::min(_restitution, mat._restitution);
		case FactorBlendOp::ARITHMETIC_MEAN:
			return 0.5f * (_restitution + mat._restitution);
		case FactorBlendOp::GEOMETRIC_MEAN:
			return _sqrt_restitution * mat._sqrt_restitution;
		case FactorBlendOp::ACTIVE:
			return _restitution;
		}
		return 0.0f;
	}

	float Material::friction_with(const Material& mat, State::FrictionType friction_type) const
	{
		if (friction_type == State::FrictionType::STATIC)
		{
			switch (blending.static_friction)
			{
			case FactorBlendOp::MINIMUM:
				return std::min(_static_friction, mat._static_friction);
			case FactorBlendOp::ARITHMETIC_MEAN:
				return 0.5f * (_static_friction + mat._static_friction);
			case FactorBlendOp::GEOMETRIC_MEAN:
				return _sqrt_static_friction * mat._sqrt_static_friction;
			case FactorBlendOp::ACTIVE:
				return _static_friction;
			}
		}
		else if (friction_type == State::FrictionType::KINETIC)
		{
			switch (blending.kinetic_friction)
			{
			case FactorBlendOp::MINIMUM:
				return std::min(_kinetic_friction, mat._kinetic_friction);
			case FactorBlendOp::ARITHMETIC_MEAN:
				return 0.5f * (_kinetic_friction + mat._kinetic_friction);
			case FactorBlendOp::GEOMETRIC_MEAN:
				return _sqrt_kinetic_friction * mat._sqrt_kinetic_friction;
			case FactorBlendOp::ACTIVE:
				return _kinetic_friction;
			}
		}
		else if (friction_type == State::FrictionType::ROLLING)
		{
			switch (blending.rolling_friction)
			{
			case FactorBlendOp::MINIMUM:
				return std::min(_rolling_friction, mat._rolling_friction);
			case FactorBlendOp::ARITHMETIC_MEAN:
				return 0.5f * (_rolling_friction + mat._rolling_friction);
			case FactorBlendOp::GEOMETRIC_MEAN:
				return _sqrt_rolling_friction * mat._sqrt_rolling_friction;
			case FactorBlendOp::ACTIVE:
				return _rolling_friction;
			}
		}
		return 0.0f;
	}

	glm::vec2 CollisionResponse::dx_teleport(float active_mass) const
	{
		glm::vec2 dx_t = mtv;
		if (dynamics->flag != DynamicsComponent::Flag::STATIC)
			dx_t *= dynamics->properties.mass() / (active_mass + dynamics->properties.mass());
		return dx_t;
	}

	float CollisionResponse::dtheta_teleport(glm::vec2 active_center_of_mass, float active_mass, float active_moi_inverse) const
	{
		float dtheta_t = math::cross(contact - active_center_of_mass, mtv);
		if (dynamics->flag != DynamicsComponent::Flag::STATIC)
			dtheta_t *= active_mass * active_moi_inverse * dynamics->properties.mass() / (active_mass + dynamics->properties.mass()); // TODO use effective mass instead?
		return dtheta_t;
	}

	void DynamicsComponent::add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& dynamics) const
	{
		collisions.emplace_back(mtv, contact, UnitVector2D(mtv), &dynamics);
	}

	// LATER pausing mechanism for RigidBody/TIME so that dt doesn't keep increasing as game is paused. + Time dilation (slo-mo).

	// TODO update DOC on resolution bias removal and penetration damping.

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
				if (material.linear_drag > 0.0f)
					state.linear_velocity *= glm::exp(-material.linear_drag * TIME.delta());
				if (flag == Flag::KINEMATIC)
					if (material.angular_drag > 0.0f)
						state.angular_velocity *= glm::exp(-material.angular_drag * TIME.delta());

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

		glm::vec2 teleport = {};
		for (const CollisionResponse& collision : collisions)
		{
			// TODO smarter accumulation of teleport than addition
			teleport += collision.dx_teleport(properties.mass());
		}

		// natural position update

		UnitVector2D teleport_axis(teleport);
		glm::vec2 dx_v = new_velocity * TIME.delta();
		float along_teleport_axis = teleport_axis.dot(dx_v);
		glm::vec2 perp_teleport_axis = dx_v - along_teleport_axis * (glm::vec2)teleport_axis;
		if (along_teleport_axis < 0.0f)
			along_teleport_axis = (1.0f - material.penetration_damping) * glm::max(along_teleport_axis, -glm::length(teleport));
		dx_v = perp_teleport_axis + along_teleport_axis * (glm::vec2)teleport_axis;
		state.position += teleport + dx_v;

		// velocity update

		state.linear_velocity = dx_v * TIME.inverse_delta() + collision_impulse * properties.mass_inverse();
		if (material.linear_drag > 0.0f)
			state.linear_velocity *= glm::exp(-material.linear_drag * TIME.delta());
	}

	void DynamicsComponent::update_colliding_angular_motion(float new_velocity, float collision_impulse) const
	{
		// collision rotation update

		float teleport = 0.0f;
		for (const CollisionResponse& collision : collisions)
		{
			// TODO smarter accumulation of teleport than addition
			teleport += collision.dtheta_teleport(properties.center_of_mass, properties.mass(), properties.moi_inverse());
		}
		teleport *= properties.mass() * properties.moi_inverse();

		// position adjustment

		// TODO fix position adjustment - teleport has a jump
		//glm::mat2 teleport_matrix = rotation_matrix_2x2(teleport);
		//teleport_matrix = glm::mat2(1.0f) - teleport_matrix;
		//glm::vec2 linear_teleport_adjustment = {};
		//for (const CollisionResponse& collision : collisions)
		//	linear_teleport_adjustment += teleport_matrix * (collision.contact - properties.center_of_mass);
		////LOG << linear_teleport_adjustment << LOG.nl;
		//LOG << teleport << "\t\t" << collisions[0].contact << LOG.nl;
		//state.position += linear_teleport_adjustment;

		// natural rotation udpate

		float dtheta_w = new_velocity * TIME.delta();
		if (above_zero(teleport))
			dtheta_w = std::min(0.0f, dtheta_w);
		else if (below_zero(teleport))
			dtheta_w = std::max(0.0f, dtheta_w);
		state.rotation += teleport + dtheta_w;

		// velocity update

		state.angular_velocity = dtheta_w * TIME.inverse_delta() + collision_impulse * properties.moi_inverse();
		if (material.angular_drag > 0.0f)
			state.angular_velocity *= glm::exp(-material.angular_drag * TIME.delta());
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
		const float cross1 = math::cross(collision.contact - properties.center_of_mass, collision.normal);
		float effective_mass_denominator = properties.mass_inverse() + properties.moi_inverse() * cross1 * cross1;
		if (collision.dynamics->flag != Flag::STATIC)
		{
			const float cross2 = math::cross(state.position + collision.contact - properties.center_of_mass - collision.dynamics->state.position, collision.normal);
			effective_mass_denominator += collision.dynamics->properties.mass_inverse() + collision.dynamics->properties.moi_inverse() * cross2 * cross2;
		}
		return 1.0f / effective_mass_denominator;
	}

	glm::vec2 DynamicsComponent::restitution_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 other_contact_velocity) const
	{
		const float restitution = material.restitution_with(collision.dynamics->material);
		if (near_zero(restitution))
			return {};

		glm::vec2 relative_velocity = linear_velocity_at(state.linear_velocity, state.angular_velocity, collision.contact - properties.center_of_mass) - other_contact_velocity;
		return std::max(-eff_mass * restitution * collision.normal.dot(relative_velocity), 0.0f) * (glm::vec2)collision.normal;
	}

	glm::vec2 DynamicsComponent::friction_impulse(const CollisionResponse& collision, float eff_mass, glm::vec2 other_contact_velocity, glm::vec2 new_linear_velocity, float new_angular_velocity) const
	{
		State::FrictionType friction_type = state.friction_type(collision.contact - properties.center_of_mass, collision.normal.get_quarter_turn(), collision.dynamics->state);
		float mu = material.friction_with(collision.dynamics->material, friction_type);
		if (col2d::near_zero(mu))
			return {};

		glm::vec2 new_relative_velocity = linear_velocity_at(new_linear_velocity, new_angular_velocity, collision.contact - properties.center_of_mass) - other_contact_velocity; // TODO use other's new velocity as well??
		glm::vec2 new_tangent_velocity = collision.normal.normal_project(new_relative_velocity);
		float new_tangent_velocity_sqrd = math::mag_sqrd(new_tangent_velocity);
		if (col2d::near_zero(new_tangent_velocity_sqrd))
			return {};


		float normal_impulse = collision.normal.dot(collision.dx_teleport(properties.mass()) * properties.mass() * TIME.inverse_delta());
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
