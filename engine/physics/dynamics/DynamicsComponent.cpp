#include "DynamicsComponent.h"

#include "core/util/Time.h"

namespace oly::physics
{
	void DynamicsComponent::on_tick()
	{
		// TODO

		// update linear velocity
		state.linear_velocity += properties.linear_acceleration * TIME.delta<float>();
		state.linear_velocity += properties.force * properties.get_inverse_mass() * TIME.delta<float>();
		for (Impulse impulse : impulses)
			state.linear_velocity += impulse.impulse * properties.get_inverse_mass();

		// update angular velocity
		state.angular_velocity += properties.angular_acceleration * TIME.delta<float>();

		// update position
		state.position += state.linear_velocity * TIME.delta<float>();

		// update rotation
		state.rotation += state.angular_velocity * TIME.delta<float>();

		impulses.clear();
	}
}
