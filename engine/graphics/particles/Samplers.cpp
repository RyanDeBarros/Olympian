#include "Samplers.h"

#include "ShaderStructs.h"

namespace oly::particles
{
	void UniformSampler::apply(internal::Sampler& sampler) const
	{
		sampler.type = internal::Sampler::UNIFORM;
	}
}
