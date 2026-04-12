#include "Spawners.h"

#include "core/util/Loader.h"

#include ".gen/keys/ParticleSystem.inl"

namespace oly::particles
{
	float ConstantParticleSpawner::spawn_debt(float time, float delta_time, float period) const
	{
		return rate * delta_time;
	}

	void ConstantParticleSpawner::overload(TOMLNode node)
	{
		io::parse_float(io::parse_key(node, _gen::keys::ParticleSystem::Rate), rate);
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
		io::parse_float(io::parse_key(node, _gen::keys::ParticleSystem::Rate), rate);
		io::parse_float(io::parse_key(node, _gen::keys::ParticleSystem::Duration), duration);
		io::parse_float(io::parse_key(node, _gen::keys::ParticleSystem::TimeOffset), time_offset);
	}
}
