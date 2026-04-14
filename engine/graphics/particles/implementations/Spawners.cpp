#include "Spawners.h"

#include "core/util/Parser.h"

#include ".gen/keys/ParticleSystem.inl"

namespace oly::particles
{
	float ConstantParticleSpawner::spawn_debt(float time, float delta_time, float period) const
	{
		return rate * delta_time;
	}

	void ConstantParticleSpawner::overload(TOMLNode node)
	{
		assets::Parser(node).optional(_gen::keys::ParticleSystem::Rate)(rate);
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
		assets::Parser parser(node);
		parser.optional(_gen::keys::ParticleSystem::Rate)(rate);
		parser.optional(_gen::keys::ParticleSystem::Duration)(duration);
		parser.optional(_gen::keys::ParticleSystem::TimeOffset)(time_offset);
	}
}
