#pragma once

#include "graphics/particles/Attribute.h"
#include "graphics/particles/AttributeGenerator.h"

namespace oly::particles
{
	struct UniformSampler1D
        : public ISampler1D
	{
        IMP_POLYMORPHIC_IMPL((UniformSampler1D));

		void apply(internal::Sampler1D& sampler) const override;
	};

	struct TiltedSampler1D
        : public ISampler1D
	{
        IMP_POLYMORPHIC_IMPL((TiltedSampler1D));

		Attribute<float> tilt;

        TiltedSampler1D(float tilt = 0.f);
        
		void apply(internal::Sampler1D& sampler) const override;
        void on_tick(const ParticleEmitter& emitter) override;
		void overload(TOMLNode node) override;
	};

	struct UniformSampler2D
        : public ISampler2D
	{
        IMP_POLYMORPHIC_IMPL((UniformSampler2D));

		void apply(internal::Sampler2D& sampler) const override;
	};

	struct UniformSampler3D
        : public ISampler3D
	{
        IMP_POLYMORPHIC_IMPL((UniformSampler3D));

		void apply(internal::Sampler3D& sampler) const override;
	};

	struct UniformSampler4D
        : public ISampler4D
	{
        IMP_POLYMORPHIC_IMPL((UniformSampler4D));

		void apply(internal::Sampler4D& sampler) const override;
	};
}
