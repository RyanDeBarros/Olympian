#include "Spawners.h"

namespace oly::particles
{
	float ConstantParticleSpawner::spawn_debt(float time, float delta_time, float period) const
	{
		return rate * delta_time;
	}

	float BurstParticleSpawner::spawn_debt(float time, float delta_time, float period) const
	{
		time -= time_offset;
		if (time >= 0.0f && time <= duration)
			return rate * delta_time;
		else
			return 0.0f;
	}
}
