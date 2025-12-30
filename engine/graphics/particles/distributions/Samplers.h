#pragma once

#include "graphics/particles/AttributeGenerator.h"
#include "graphics/particles/Attribute.h"
#include "graphics/particles/distributions/Enums.h"

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
		Attribute<DirectionEnum> direction;
		Attribute<float> tilt;

		TiltedSampler1D(DirectionEnum direction = DirectionEnum::LEFT, float tilt = 1.0f) : direction(direction), tilt(tilt) {}

		void apply(internal::Sampler1D& sampler) const override;

		void on_tick(const ParticleEmitter& emitter)
		{
			direction.on_tick(emitter);
			tilt.on_tick(emitter);
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
