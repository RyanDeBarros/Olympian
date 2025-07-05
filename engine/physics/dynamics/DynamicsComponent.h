#pragma once

#include "core/base/TransformerExposure.h"
#include "core/base/SimpleMath.h"

namespace oly::physics
{
	struct Material
	{
		BoundedFloat<0.0f, 1.0f> restitution = 0.3f;
		PositiveFloat static_friction = 0.0f;
		PositiveFloat dynamic_friction = 0.0f;
		PositiveFloat linear_drag = 0.0f;
		PositiveFloat angular_drag = 0.0f;
		std::optional<PositiveFloat> density = std::nullopt;
	};

	struct State
	{
		glm::vec2 position = {};
		float rotation = 0.0f;
		glm::vec2 linear_velocity = {};
		float angular_velocity = 0.0f;
	};

	class Properties
	{
		PositiveFloat mass = 1.0f;
		PositiveFloat inverse_mass = 1.0f;
		// moment of inertia
		PositiveFloat moi = 1.0f;
		PositiveFloat inverse_moi = 1.0f;

	public:
		glm::vec2 force = {};
		float torque = 0.0f;
		glm::vec2 linear_acceleration = {};
		float angular_acceleration = 0.0f;

		float get_mass() const { return mass.get(); }
		void set_mass(float m) { mass.set(m); inverse_mass.set(1.0f / mass.get()); }
		float get_inverse_mass() const { return inverse_mass.get(); }

		float get_moi() const { return moi.get(); }
		void set_moi(float m) { moi.set(m); inverse_moi.set(1.0f / moi.get()); }
		float get_inverse_moi() const { return inverse_moi.get(); }
	};

	class DynamicsComponent
	{
		State state;

	public:
		Material material;
		Properties properties;

		struct Impulse
		{
			glm::vec2 impulse = {};
			glm::vec2 relative_position = {};
		};
		std::vector<Impulse> impulses;

		void on_tick();
	};
}
