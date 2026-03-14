#include "Samplers.h"

#include "graphics/particles/ShaderStructs.h"

namespace oly::particles
{
	void UniformSampler1D::apply(internal::Sampler1D& sampler) const
	{
		sampler.type = internal::Sampler1D::Uniform;
	}

	void TiltedSampler1D::apply(internal::Sampler1D& sampler) const
	{
		sampler.type = internal::Sampler1D::Tilted;
		sampler.params[0] = tilt.value;
	}

	void UniformSampler2D::apply(internal::Sampler2D& sampler) const
	{
		sampler.type = internal::Sampler2D::Uniform;
	}

	void UniformSampler3D::apply(internal::Sampler3D& sampler) const
	{
		sampler.type = internal::Sampler3D::Uniform;
	}

	void UniformSampler4D::apply(internal::Sampler4D& sampler) const
	{
		sampler.type = internal::Sampler4D::Uniform;
	}
}
