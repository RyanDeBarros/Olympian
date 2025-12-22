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

	void UniformSampler2D::apply(internal::Sampler2D& sampler) const
	{
		sampler.type = internal::Sampler2D::UNIFORM;
	}

	void UniformSampler3D::apply(internal::Sampler3D& sampler) const
	{
		sampler.type = internal::Sampler3D::UNIFORM;
	}

	void UniformSampler4D::apply(internal::Sampler4D& sampler) const
	{
		sampler.type = internal::Sampler4D::UNIFORM;
	}
}
