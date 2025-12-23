#pragma once

#include "graphics/particles/AttributeGenerator.h"

namespace oly::particles
{
	struct ConstantParticleSpawner : public IParticleSpawner
	{
		float rate = 0.0f;

		ConstantParticleSpawner(float rate = 10.0f) : rate(rate) {}

		float spawn_debt(float time, float delta_time, float period) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(ConstantParticleSpawner);
	};

	struct BurstParticleSpawner : public IParticleSpawner
	{
		float rate = 0.0f;
		float duration = 0.0f;
		float time_offset = 0.0f;

		BurstParticleSpawner(float rate = 100.0f, float duration = 0.1f, float time_offset = 0.0f) : rate(rate), duration(duration), time_offset(time_offset) {}

		float spawn_debt(float time, float delta_time, float period) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(BurstParticleSpawner);
	};
}
