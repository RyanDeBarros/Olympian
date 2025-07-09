#include "DynamicsComponent.h"

#include "core/util/Time.h"
#include "core/math/Geometry.h"
#include "physics/dynamics/RigidBody.h"

namespace oly::physics
{
	glm::vec2 State::linear_velocity_at(glm::vec2 contact) const
	{
		return linear_velocity + angular_velocity * glm::vec2{ -contact.y, contact.x };
	}

	State::FrictionType State::friction_type(glm::vec2 contact, UnitVector2D tangent, const State& other, PositiveFloat speed_threshold) const
	{
		return near_zero(tangent.dot(linear_velocity_at(contact) - other.linear_velocity_at(contact + position - other.position)), speed_threshold) ? FrictionType::STATIC : FrictionType::KINEMATIC;
	}

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

	float Material::restitution_with(const Material& mat) const
	{
		return std::min(restitution, mat.restitution); // TODO other strategies
	}

	float Material::friction_with(glm::vec2 contact, UnitVector2D tangent, const State& state, const Material& mat, const State& other_state, PositiveFloat speed_threshold) const
	{
		State::FrictionType friction_type = state.friction_type(contact, tangent, other_state, speed_threshold);
		if (friction_type == State::FrictionType::STATIC)
			return _sqrt_static_friction * mat._sqrt_static_friction; // TODO other strategies;
		else // TODO else if == KINEMATIC, then else ROLLING
			return _sqrt_kinematic_friction * mat._sqrt_kinematic_friction; // TODO other strategies;
	}

	void DynamicsComponent::add_collision(glm::vec2 mtv, glm::vec2 contact, const DynamicsComponent& dynamics) const
	{
		if (dynamics.flag == Flag::STATIC)
			static_collisions.emplace_back(mtv, contact, UnitVector2D(mtv), &dynamics);
		else if (dynamics.flag == Flag::KINEMATIC)
			kinematic_collisions.emplace_back(mtv, contact, UnitVector2D(mtv), &dynamics);
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
			for (const CollisionResponse& collision : static_collisions)
				dtheta_t += math::cross(collision.contact, collision.mtv);
			for (const CollisionResponse& collision : kinematic_collisions)
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

				float restitution = material.restitution_with(kinematic_collision.dynamics->material);
				float _j_r = -(1.0f + restitution) / (properties.mass_inverse() + kinematic_collision.dynamics->properties.mass_inverse())
					* kinematic_collision.normal.dot(state.linear_velocity - kinematic_collision.dynamics->state.linear_velocity);
				if (_j_r < 0.0f)
					_j_r = 0.0f;
				glm::vec2 j_r = _j_r * (glm::vec2)kinematic_collision.normal;

				float mu = material.friction_with(kinematic_collision.contact, kinematic_collision.normal.get_quarter_turn(),
					state, kinematic_collision.dynamics->material, kinematic_collision.dynamics->state);
				glm::vec2 j_f = -mu * glm::length(j_r) * (glm::vec2)UnitVector2D(kinematic_collision.mtv).normal_project(state.linear_velocity - kinematic_collision.dynamics->state.linear_velocity);

				linear_impulse = j_r + j_f;
				angular_impulse = math::cross(kinematic_collision.contact, linear_impulse);
			}
		}
		else
		{
			const size_t N = kinematic_collisions.size();
			std::vector<float> C(N);

			for (size_t i = 0; i < N; ++i)
			{
				C[i] = (1.0f + material.restitution_with(kinematic_collisions[i].dynamics->material))
					* kinematic_collisions[i].normal.dot(state.linear_velocity - kinematic_collisions[i].dynamics->state.linear_velocity);
				for (const CollisionResponse& static_collision : static_collisions)
					C[i] -= (1.0f + material.restitution_with(static_collision.dynamics->material))
						* static_collision.normal.dot(state.linear_velocity) * static_collision.normal.dot(kinematic_collisions[i].normal);

				C[i] *= -properties.mass() * kinematic_collisions[i].dynamics->properties.mass();
			}

			glm::vec2 c_sum = {};
			for (size_t i = 0; i < N; ++i)
				c_sum += C[i] * (glm::vec2)kinematic_collisions[i].normal;

			float gamma = properties.mass() * properties.mass();

			float mass_sum = 0.0f;
			for (size_t k = 0; k < N; ++k)
				mass_sum += kinematic_collisions[k].dynamics->properties.mass();
			gamma += properties.mass() * mass_sum;

			float outer_sum = 0.0f;
			for (size_t k = 0; k < N; ++k)
			{
				float inner_sum = 0.0f;
				for (size_t l = 0; l < N; ++l)
					inner_sum += kinematic_collisions[l].dynamics->properties.mass() * kinematic_collisions[l].normal.y()
						* math::cross(kinematic_collisions[k].normal, kinematic_collisions[l].normal);
				outer_sum += kinematic_collisions[k].dynamics->properties.mass() * kinematic_collisions[k].normal.x();
			}
			gamma += outer_sum;

			const float gamma_inverse = 1.0f / gamma;

			std::vector<float> j(N);

			for (size_t i = 0; i < N; ++i)
			{
				j[i] = -C[i] * properties.mass_inverse();
				j[i] -= kinematic_collisions[i].dynamics->properties.mass() * gamma_inverse * kinematic_collisions[i].normal.dot(c_sum);

				float inner_sum = 0.0f;
				for (size_t k = 0; k < N; ++k)
					inner_sum += kinematic_collisions[k].dynamics->properties.mass() * math::cross(kinematic_collisions[i].normal,
						kinematic_collisions[k].normal) * math::cross(c_sum, kinematic_collisions[k].normal);

				j[i] -= kinematic_collisions[i].dynamics->properties.mass() * gamma_inverse * properties.mass_inverse() * inner_sum;
				if (j[i] < 0.0f)
					j[i] = 0.0f;
			}

			glm::vec2 j_r = {};
			for (size_t i = 0; i < N; ++i)
				j_r += j[i] * (glm::vec2)kinematic_collisions[i].normal;

			float h_r = 0.0f;
			for (size_t i = 0; i < N; ++i)
				h_r += math::cross(kinematic_collisions[i].contact, j[i] * (glm::vec2)kinematic_collisions[i].normal);

			std::vector<glm::vec2> jf(N);
			for (size_t i = 0; i < N; ++i)
			{
				float mu = material.friction_with(kinematic_collisions[i].contact, kinematic_collisions[i].normal.get_quarter_turn(),
					state, kinematic_collisions[i].dynamics->material, kinematic_collisions[i].dynamics->state);
				jf[i] = -mu * j[i] * (glm::vec2)kinematic_collisions[i].normal.normal_project(state.linear_velocity - kinematic_collisions[i].dynamics->state.linear_velocity);
			}

			linear_impulse = j_r;
			for (size_t i = 0; i < N; ++i)
				linear_impulse += jf[i];

			angular_impulse = h_r;
			for (size_t i = 0; i < N; ++i)
				angular_impulse += math::cross(kinematic_collisions[i].contact, jf[i]);
		}

		for (const CollisionResponse& static_collision : static_collisions)
		{
			float restitution = material.restitution_with(static_collision.dynamics->material);
			float _j_r = -properties.mass() * (1.0f + material.restitution) * static_collision.normal.dot(state.linear_velocity);
			if (_j_r < 0.0f)
				_j_r = 0.0f;
			glm::vec2 j_r = _j_r * (glm::vec2)static_collision.normal;

			float mu = material.friction_with(static_collision.contact, static_collision.normal.get_quarter_turn(),
				state, static_collision.dynamics->material, static_collision.dynamics->state);
			glm::vec2 j_f = -mu * glm::length(j_r) * (glm::vec2)static_collision.normal.normal_project(state.linear_velocity);

			linear_impulse += j_r + j_f;
			angular_impulse += math::cross(static_collision.contact, linear_impulse);
		}
	}
}
