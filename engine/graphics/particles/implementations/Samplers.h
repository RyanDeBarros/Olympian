#pragma once

#include "graphics/particles/AttributeGenerator.h"
#include "graphics/particles/Attribute.h"

namespace oly::particles
{
	struct UniformSampler1D : public ISampler1D
	{
		void apply(internal::Sampler1D& sampler) const override;
		
		void on_tick(const ParticleEmitter& emitter) {}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(UniformSampler1D);
	};

	struct TiltedSampler1D : public ISampler1D
	{
		Attribute<float> tilt;

		TiltedSampler1D(float tilt = 0.0f) : tilt(tilt) {}

		void apply(internal::Sampler1D& sampler) const override;

		void on_tick(const ParticleEmitter& emitter)
		{
			tilt.on_tick(emitter);
		}

		void overload(TOMLNode node) override
		{
			tilt.overload(node["tilt"]);
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(TiltedSampler1D);
	};

	struct UniformSampler2D : public ISampler2D
	{
		void apply(internal::Sampler2D& sampler) const override;

		void on_tick(const ParticleEmitter& emitter) {}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(UniformSampler2D);
	};

	struct UniformSampler3D : public ISampler3D
	{
		void apply(internal::Sampler3D& sampler) const override;

		void on_tick(const ParticleEmitter& emitter) {}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(UniformSampler3D);
	};

	struct UniformSampler4D : public ISampler4D
	{
		void apply(internal::Sampler4D& sampler) const override;

		void on_tick(const ParticleEmitter& emitter) {}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(UniformSampler4D);
	};
}
