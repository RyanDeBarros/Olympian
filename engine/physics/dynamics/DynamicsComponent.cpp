#include "DynamicsComponent.h"

#include "core/util/Time.h"
#include "core/math/Geometry.h"
#include "physics/dynamics/RigidBody.h"

namespace oly::physics
{
	glm::vec2 Properties::dv_psi() const
	{
		glm::vec2 dv = {};

		glm::vec2 accumul_net_lin_accel = net_linear_acceleration;
		for (AppliedAcceleration accel : applied_accelerations)
			accumul_net_lin_accel += accel.acceleration;
		dv += accumul_net_lin_accel * TIME.delta<>();

		glm::vec2 accumul_net_force = net_force;
		for (AppliedForce force : applied_forces)
			accumul_net_force += force.force;
		dv += accumul_net_force * TIME.delta<>() * _mass_inverse.get();

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

		float net_applied_angular_acceleration = 0.0f;
		for (AppliedAcceleration accel : applied_accelerations)
			net_applied_angular_acceleration += math::cross(accel.contact, accel.acceleration);
		dw += net_applied_angular_acceleration * TIME.delta<>() * _mass * _moi_inverse;

		float net_applied_torque = 0.0f;
		for (AppliedForce force : applied_forces)
			net_applied_torque += math::cross(force.contact, force.force);
		dw += net_applied_torque * TIME.delta<>() * _moi_inverse;

		float net_applied_angular_impulse = 0.0f;
		for (AppliedImpulse impulse : applied_impulses)
			net_applied_angular_impulse += math::cross(impulse.contact, impulse.impulse);
		dw += net_applied_angular_impulse * _moi_inverse;

		return dw;
	}

	void DynamicsComponent::add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& other) const
	{
		if (other.flag == Flag::STATIC)
			static_collisions.emplace_back(mtv, contact, other);
		else if (other.flag == Flag::KINEMATIC)
			kinematic_collisions.emplace_back(mtv, contact, other);
	}

	void DynamicsComponent::on_tick() const
	{
		// 1. prep collision response

		glm::vec2 j_c = {};
		float h_c = 0.0f;
		compute_collision_response(j_c, h_c);

		// 2. update linear velocity

		glm::vec2 dv = properties.dv_psi() + j_c * properties.mass_inverse();
		state.linear_velocity += state.linear_velocity + dv;
		state.linear_velocity *= glm::exp(-material.linear_drag * TIME.delta<>() * properties.mass_inverse());

		// 3. update angular velocity

		float dw = properties.dw_psi() + h_c * properties.moi_inverse();
		state.angular_velocity += dw;
		state.angular_velocity *= glm::exp(-material.angular_drag * TIME.delta<>() * properties.moi_inverse());

		// 4. update position

		glm::vec2 dx = {};
		if (material.resolution_bias > 0.0f)
			dx += material.resolution_bias.get()* state.linear_velocity* TIME.delta<>();
		if (material.resolution_bias < 1.0f)
		{
			glm::vec2 dx_t = {};
			for (const CollisionResponse& collision : static_collisions)
				dx_t += collision.mtv;
			for (const CollisionResponse& collision : kinematic_collisions)
				dx_t += collision.mtv;

			dx += (1.0f - material.resolution_bias.get()) * dx_t;
		}

		state.position += dx;

		// 5. update rotation

		float dtheta = 0.0f;
		if (material.resolution_bias > 0.0f)
			dtheta += material.resolution_bias.get() * state.angular_velocity * TIME.delta<>();
		if (material.resolution_bias < 1.0f)
		{
			float dtheta_t = 0.0f;
			for (CollisionResponse collision : static_collisions)
				dtheta_t += math::cross(collision.contact, collision.mtv);
			for (CollisionResponse collision : kinematic_collisions)
				dtheta_t += math::cross(collision.contact, collision.mtv);

			dtheta += (1.0f - material.resolution_bias.get()) * dtheta_t * properties.mass() * properties.moi_inverse();
		}

		state.position += dtheta;

		// 6. clean up

		static_collisions.clear();
		kinematic_collisions.clear();
	}

	void DynamicsComponent::compute_collision_response(glm::vec2& linear_impulse, float& angular_impulse) const
	{
		linear_impulse = {};
		angular_impulse = 0.0f;
		if (kinematic_collisions.size() <= 1)
		{
			if (kinematic_collisions.size() == 1)
			{
				const CollisionResponse& kinematic_collision = kinematic_collisions[0];

				float restitution = glm::min(material.restitution, kinematic_collision.other.material.restitution); // TODO other strategies
				float _j_r = -(1.0f + restitution) / (math::mag_sqrd(kinematic_collision.mtv) * (properties.mass_inverse() + kinematic_collision.other.properties.mass_inverse()))
					* glm::dot(state.linear_velocity - kinematic_collision.other.state.linear_velocity, kinematic_collision.mtv);
				if (_j_r < 0.0f)
					_j_r = 0.0f;
				glm::vec2 j_r = _j_r * kinematic_collision.mtv;

				float mu = material.sqrt_static_friction() * kinematic_collision.other.material.sqrt_static_friction(); // TODO select the correct coefficient of friction
				glm::vec2 j_f = -mu * glm::length(j_r) * (glm::vec2)UnitVector2D(kinematic_collision.mtv).normal_project(state.linear_velocity - kinematic_collision.other.state.linear_velocity);

				linear_impulse = j_r + j_f;
				angular_impulse = math::cross(kinematic_collision.contact, linear_impulse);
			}

			for (const CollisionResponse& static_collision : static_collisions)
			{
				float restitution = glm::min(material.restitution, static_collision.other.material.restitution); // TODO other strategies
				float _j_r = -properties.mass() * (1.0f + material.restitution) / math::mag_sqrd(static_collision.mtv) * glm::dot(state.linear_velocity, static_collision.mtv);
				if (_j_r < 0.0f)
					_j_r = 0.0f;
				glm::vec2 j_r = _j_r * static_collision.mtv;

				float mu = material.sqrt_static_friction() * static_collision.other.material.sqrt_static_friction(); // TODO select the correct coefficient of friction. other strategies
				glm::vec2 j_f = -mu * glm::length(j_r) * (glm::vec2)UnitVector2D(static_collision.mtv).normal_project(state.linear_velocity);

				linear_impulse += j_r + j_f;
				angular_impulse += math::cross(static_collision.contact, linear_impulse);
			}
		}
		else
		{
			// TODO
		}
	}
}
