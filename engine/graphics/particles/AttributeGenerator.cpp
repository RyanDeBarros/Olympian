#include "AttributeGenerator.h"

#include "ShaderStructs.h"

namespace oly::particles
{
	void AttributeGenerator1D::apply(internal::Generator1D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void AttributeGenerator2D::apply(internal::Generator2D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void AttributeGenerator3D::apply(internal::Generator3D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}

	void AttributeGenerator4D::apply(internal::Generator4D& generator) const
	{
		sampler->apply(generator.sampler);
		domain->apply(generator.domain);
	}
}
