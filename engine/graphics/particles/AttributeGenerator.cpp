#include "AttributeGenerator.h"

#include "ShaderStructs.h"

namespace oly::particles
{
	void AttributeGenerator::apply(internal::Generator& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}
}
