#pragma once

#include "graphics/particles/Attribute.h"

namespace oly::particles::ops
{
	struct SineWave1D
        : public IAttributeOperation
	{
        IMP_POLYMORPHIC_IMPL((SineWave1D));

    public:
		// attribute = a * sin(b * t - k) + c
		float a = 1.f;
		float b = 1.f;
		float k = 0.f;
		float c = 0.f;

		SineWave1D() = default;
        SineWave1D(float a, float b, float k, float c);

		void op(const ParticleEmitter& emitter, AttributeSpan attribute) const override;

		static imp::poly<SineWave1D> load(TOMLNode node);
	};

	struct Polarization2D
        : public IAttributeOperation
	{
        IMP_POLYMORPHIC_IMPL((Polarization2D));

    public:
		float amplitude = 1.f;
		float time_offset = 0.f;

		Polarization2D() = default;
        Polarization2D(float amplitude, float time_offset);

		void op(const ParticleEmitter& emitter, AttributeSpan attribute) const override;

		static imp::poly<Polarization2D> load(TOMLNode node);
	};
}
