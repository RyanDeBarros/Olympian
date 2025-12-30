#pragma once

#include "graphics/particles/Attribute.h"

namespace oly::particles::operations
{
	struct SineWave1D : public IAttributeOperation<float>
	{
		// attribute = a * sin(b * t - k) + c
		float a = 1.0f;
		float b = 1.0f;
		float k = 0.0f;
		float c = 0.0f;

		SineWave1D() = default;
		SineWave1D(float a, float b, float k, float c) : a(a), b(b), k(k), c(c) {}

		void op(const ParticleEmitter& emitter, float& attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SineWave1D);
	};

	struct Polarization2D : public IAttributeOperation<glm::vec2>
	{
		float amplitude = 1.0f;
		float time_offset = 0.0f;

		Polarization2D() = default;
		Polarization2D(float amplitude, float time_offset) : amplitude(amplitude), time_offset(time_offset) {}

		void op(const ParticleEmitter& emitter, glm::vec2& attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(Polarization2D);
	};
}
