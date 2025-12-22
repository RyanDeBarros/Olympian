#pragma once

#include "graphics/particles/AttributeGenerator.h"

namespace oly::particles
{
	struct UniformSampler : public ISampler
	{
		void apply(internal::Sampler& sampler) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(UniformSampler);
	};
}
