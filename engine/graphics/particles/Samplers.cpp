#include "Samplers.h"

#include "ShaderStructs.h"

namespace oly::particles
{
	void UniformSampler1D::apply(internal::Sampler1D& sampler) const
	{
		sampler.type = internal::Sampler1D::UNIFORM;
	}

	void TiltedSampler1D::apply(internal::Sampler1D& sampler) const
	{
		sampler.type = internal::Sampler1D::TILTED;
		switch (direction)
		{
		case Direction::RIGHT:
			sampler.params[0] = glm::abs(tilt);
			break;
		case Direction::LEFT:
			sampler.params[0] = -glm::abs(tilt);
			break;
		case Direction::NONE:
			sampler.params[0] = 0.0f;
			break;
		}
	}
}
