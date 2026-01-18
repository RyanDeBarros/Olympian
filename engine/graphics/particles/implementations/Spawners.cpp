#include "Spawners.h"

#include "core/util/Loader.h"

namespace oly::particles
{
	float ConstantParticleSpawner::spawn_debt(float time, float delta_time, float period) const
	{
		return rate * delta_time;
	}

	void ConstantParticleSpawner::overload(TOMLNode node)
	{
		io::parse_float(node["rate"], rate);
	}

	float BurstParticleSpawner::spawn_debt(float time, float delta_time, float period) const
	{
		time -= time_offset;
		if (time >= 0.0f && time <= duration)
			return rate * delta_time;
		else
			return 0.0f;
	}

	void BurstParticleSpawner::overload(TOMLNode node)
	{
		io::parse_float(node["rate"], rate);
		io::parse_float(node["duration"], duration);
		io::parse_float(node["time_offset"], time_offset);
	}
}
