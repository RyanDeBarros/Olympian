#include "DynamicsComponent.h"

#include "core/util/Time.h"
#include "core/math/Geometry.h"
#include "physics/dynamics/RigidBody.h"

namespace oly::physics
{
	static FactorBlendOp restitution_blend_op = FactorBlendOp::MINIMUM;
	static FactorBlendOp friction_blend_op = FactorBlendOp::GEOMETRIC_MEAN;

	void set_restitution_blend_op(FactorBlendOp op)
	{
		restitution_blend_op = op;
	}

	void set_friction_blend_op(FactorBlendOp op)
	{
		friction_blend_op = op;
	}

	glm::vec2 linear_velocity_at(glm::vec2 linear_velocity, float angular_velocity, glm::vec2 contact)
	{
		return linear_velocity + angular_velocity * glm::vec2{ -contact.y, contact.x };
	}

	State::FrictionType State::friction_type(glm::vec2 contact, UnitVector2D tangent, const State& other, PositiveFloat speed_threshold) const
	{
		bool sliding = !near_zero(tangent.dot(linear_velocity_at(linear_velocity, angular_velocity, contact)
			- linear_velocity_at(other.linear_velocity, other.angular_velocity, contact + position - other.position)), speed_threshold);
		if (sliding)
			return FrictionType::KINEMATIC;
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
		dv += (net_linear_acceleration + _net_linear_applied_acceleration) * TIME.delta<>();

		if (dirty_linear_applied_forces)
		{
			dirty_linear_applied_forces = false;
			_net_linear_applied_force = {};
			for (AppliedForce force : applied_forces)
				_net_linear_applied_force += force.force;
		}
		dv += (net_force + _net_linear_applied_force) * TIME.delta<>() * _mass_inverse.get();

		glm::vec2 accumul_net_lin_impulse = net_linear_impulse;
		for (AppliedImpulse impulse : applied_impulses)
			accumul_net_lin_impulse += impulse.impulse;
		dv += accumul_net_lin_impulse * _mass_inverse.get();

		return dv;
	}

	float Properties::dw_psi() const
	{
		float dw = 0.0f;

		dw += net_angular_acceleration * TIME.delta<>();

		dw += net_torque * TIME.delta<>() * _moi_inverse;

		dw += net_angular_impulse * _moi_inverse;

		if (dirty_angular_applied_accelerations)
		{
			dirty_angular_applied_accelerations = false;
			_net_angular_applied_acceleration = 0.0f;
			for (AppliedAcceleration accel : applied_accelerations)
				_net_angular_applied_acceleration += math::cross(accel.contact, accel.acceleration);
		}
		dw += _net_angular_applied_acceleration * TIME.delta<>() * _mass * _moi_inverse;

		if (dirty_angular_applied_forces)
		{
			dirty_angular_applied_forces = false;
			_net_angular_applied_force = 0.0f;
			for (AppliedForce force : applied_forces)
				_net_angular_applied_force += math::cross(force.contact, force.force);
		}
		dw += _net_angular_applied_force * TIME.delta<>() * _moi_inverse;

		float net_applied_angular_impulse = 0.0f;
		for (AppliedImpulse impulse : applied_impulses)
			net_applied_angular_impulse += math::cross(impulse.contact, impulse.impulse);
		dw += net_applied_angular_impulse * _moi_inverse;

		return dw;
	}

	float Material::restitution_with(const Material& mat) const
	{
		switch (restitution_blend_op)
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

	float Material::friction_with(glm::vec2 contact, UnitVector2D tangent, const State& state, const Material& mat, const State& other_state, PositiveFloat speed_threshold) const
	{
		State::FrictionType friction_type = state.friction_type(contact, tangent, other_state, speed_threshold);
		if (friction_type == State::FrictionType::STATIC)
		{
			switch (friction_blend_op)
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
		else if (friction_type == State::FrictionType::KINEMATIC)
		{
			switch (friction_blend_op)
			{
			case FactorBlendOp::MINIMUM:
				return std::min(_kinematic_friction, mat._kinematic_friction);
			case FactorBlendOp::ARITHMETIC_MEAN:
				return 0.5f * (_kinematic_friction + mat._kinematic_friction);
			case FactorBlendOp::GEOMETRIC_MEAN:
				return _sqrt_kinematic_friction * mat._sqrt_kinematic_friction;
			case FactorBlendOp::ACTIVE:
				return _kinematic_friction;
			}
		}
		else if (friction_type == State::FrictionType::ROLLING)
		{
			switch (friction_blend_op)
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

	void DynamicsComponent::add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& dynamics) const
	{
		if (dynamics.flag == Flag::STATIC)
			static_collisions.emplace_back(mtv, contact, UnitVector2D(mtv), &dynamics);
		else if (dynamics.flag == Flag::KINEMATIC)
			kinematic_collisions.emplace_back(mtv, contact, UnitVector2D(mtv), &dynamics);
	}

	// LATER pausing mechanism for RigidBody/TIME so that dt doesn't keep increasing as game is paused. + Time dilation (slo-mo).

	void DynamicsComponent::on_tick() const
	{
		if (flag == Flag::KINEMATIC)
		{
			// 1. update velocity from non-collision stimuli

			glm::vec2 new_linear_velocity = state.linear_velocity + properties.dv_psi();
			float new_angular_velocity = state.angular_velocity + properties.dw_psi();

			// 2. compute collision response

			glm::vec2 j_c = {};
			float h_c = 0.0f;
			compute_collision_response(j_c, h_c, new_linear_velocity, new_angular_velocity);

			// 3. update velocity from collision stimuli

			state.linear_velocity = new_linear_velocity + j_c * properties.mass_inverse();
			state.angular_velocity = new_angular_velocity + h_c * properties.moi_inverse();

			// 4. apply drag

			if (material.linear_drag > 0.0f)
				state.linear_velocity *= glm::exp(-material.linear_drag * TIME.delta<>());
			if (material.angular_drag > 0.0f)
				state.angular_velocity *= glm::exp(-material.angular_drag * TIME.delta<>());

			// 5. update position

			glm::vec2 dx = {};
			if (is_colliding())
			{
				if (material.resolution_bias > 0.0f)
					dx += material.resolution_bias.get() * state.linear_velocity * TIME.delta<>();
				if (material.resolution_bias < 1.0f)
				{
					glm::vec2 dx_t = {};
					for (const CollisionResponse& collision : static_collisions)
						dx_t += collision.mtv;
					for (const CollisionResponse& collision : kinematic_collisions)
						dx_t += collision.dynamics->properties.mass() * collision.mtv / (properties.mass() + collision.dynamics->properties.mass());

					dx += (1.0f - material.resolution_bias.get()) * dx_t;
				}
			}
			else
				dx += state.linear_velocity * TIME.delta<>();

			state.position += dx;

			// 6. update rotation

			float dtheta = 0.0f;
			if (is_colliding())
			{
				if (material.resolution_bias > 0.0f)
					dtheta += material.resolution_bias.get() * state.angular_velocity * TIME.delta<>();
				if (material.resolution_bias < 1.0f)
				{
					float dtheta_t = 0.0f;
					for (const CollisionResponse& collision : static_collisions)
						dtheta_t += math::cross(collision.contact, collision.mtv);
					for (const CollisionResponse& collision : kinematic_collisions)
						dtheta_t += properties.mass() * collision.dynamics->properties.mass() * math::cross(collision.contact, collision.mtv) * properties.moi_inverse()
							/ (properties.mass() + collision.dynamics->properties.mass());

					dtheta += (1.0f - material.resolution_bias.get()) * dtheta_t * properties.mass() * properties.moi_inverse();
				}
			}
			else
				dtheta += state.angular_velocity * TIME.delta<>();

			state.rotation += dtheta;
		}

		// 7. clean up

		properties.applied_impulses.clear();
		properties.net_linear_impulse = {};
		properties.net_angular_impulse = 0.0f;
		static_collisions.clear();
		kinematic_collisions.clear();
	}

	void DynamicsComponent::compute_collision_response(glm::vec2& linear_impulse, float& angular_impulse, const glm::vec2 new_linear_velocity, const float new_angular_velocity) const
	{
		// TODO cache kinematic impulse in other dynamics component so it doesn't need to recalculate. use map of pointer->impulse that's cleared after compute_collision_response(). Same goes for resolution bias teleportation and friction.

		linear_impulse = {};
		angular_impulse = 0.0f;
		for (const CollisionResponse& collision : static_collisions)
		{
			float cross = math::cross(collision.contact, collision.normal);
			float effective_mass = 1.0f / (properties.mass_inverse() + properties.moi_inverse() * cross * cross);
			float restitution = material.restitution_with(collision.dynamics->material);
			float j = -effective_mass * (1.0f + restitution) * collision.normal.dot(linear_velocity_at(state.linear_velocity, state.angular_velocity, collision.contact));
			if (j < 0.0f)
				j = 0.0f;
			glm::vec2 j_r = j * (glm::vec2)collision.normal;
			glm::vec2 impulse = j_r;

			glm::vec2 new_tangent_velocity = collision.normal.normal_project(linear_velocity_at(new_linear_velocity, new_angular_velocity, collision.contact));
			float new_tangent_velocity_sqrd = math::mag_sqrd(new_tangent_velocity);
			if (!col2d::near_zero(new_tangent_velocity_sqrd))
			{
				float mu = material.friction_with(collision.contact, collision.normal.get_quarter_turn(), state, collision.dynamics->material, collision.dynamics->state);
				float friction = std::min(mu * j, effective_mass * glm::sqrt(new_tangent_velocity_sqrd));
				glm::vec2 j_f = -glm::normalize(new_tangent_velocity) * friction;
				impulse += j_f;
			}

			linear_impulse += impulse;
			angular_impulse += math::cross(collision.contact, impulse);
		}
		for (const CollisionResponse& collision : kinematic_collisions)
		{
			glm::vec2 other_contact = state.position + collision.contact - collision.dynamics->state.position;
			float cross1 = math::cross(collision.contact, collision.normal);
			float cross2 = math::cross(other_contact, collision.normal);
			float effective_mass = 1.0f / (properties.mass_inverse() + collision.dynamics->properties.mass_inverse()
				+ properties.moi_inverse() * cross1 * cross1 + collision.dynamics->properties.moi_inverse() * cross2 * cross2);
			float restitution = material.restitution_with(collision.dynamics->material);
			float j = -effective_mass * (1.0f + restitution) * collision.normal.dot(linear_velocity_at(state.linear_velocity, state.angular_velocity, collision.contact)
				- linear_velocity_at(collision.dynamics->state.linear_velocity, collision.dynamics->state.angular_velocity, other_contact));
			if (j < 0.0f)
				j = 0.0f;
			glm::vec2 j_r = j * (glm::vec2)collision.normal;
			glm::vec2 impulse = j_r;

			glm::vec2 new_relative_velocity = linear_velocity_at(new_linear_velocity, new_angular_velocity, collision.contact)
				- linear_velocity_at(collision.dynamics->state.linear_velocity, collision.dynamics->state.angular_velocity, other_contact); // TODO use other's new velocity as well??
			glm::vec2 new_tangent_velocity = collision.normal.normal_project(linear_velocity_at(new_linear_velocity, new_angular_velocity, collision.contact));
			float new_tangent_velocity_sqrd = math::mag_sqrd(new_tangent_velocity);
			if (!col2d::near_zero(new_tangent_velocity_sqrd))
			{
				float mu = material.friction_with(collision.contact, collision.normal.get_quarter_turn(), state, collision.dynamics->material, collision.dynamics->state);
				float friction = std::min(mu * j, effective_mass * glm::sqrt(new_tangent_velocity_sqrd));
				glm::vec2 j_f = -glm::normalize(new_tangent_velocity) * friction;
				impulse += j_f;
			}

			linear_impulse += impulse;
			angular_impulse += math::cross(collision.contact, impulse);
		}
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
