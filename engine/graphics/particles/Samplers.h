#pragma once

#include "graphics/particles/AttributeGenerator.h"

namespace oly::particles
{
	struct UniformSampler1D : public ISampler1D
	{
		void apply(internal::Sampler1D& sampler) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(UniformSampler1D);
	};

	struct TiltedSampler1D : public ISampler1D
	{
		enum class Direction
		{
			RIGHT,
			LEFT,
			NONE
		} direction;

		float tilt;

		TiltedSampler1D(Direction direction = Direction::LEFT, float tilt = 1.0f) : direction(direction), tilt(tilt) {}

		void apply(internal::Sampler1D& sampler) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(TiltedSampler1D);
	};
}
