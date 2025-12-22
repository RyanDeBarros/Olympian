#include "AttributeGenerator.h"

#include "ShaderStructs.h"

namespace oly::particles
{
	void AttributeGenerator1D::apply(internal::Generator1D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}
}
