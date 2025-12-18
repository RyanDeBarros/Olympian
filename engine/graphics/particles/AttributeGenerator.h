#pragma once

#include "graphics/particles/samplers/Sampler.h"
#include "graphics/particles/domains/Domain.h"
// TODO v6 Polymorphic.h should live in core/types, not core/containers.
#include "core/containers/Polymorphic.h"

namespace oly::particles
{
	struct AttributeGeneratorChain
	{
		Polymorphic<samplers::ISampler> sampler;
		Polymorphic<domains::IDomain> domain;

		void validate() const;

		void generate(ConstFloatSpan state_input, FloatSpan output) const;

	private:
		void generate(ConstFloatSpan state_input, ConstFloatSpan random_input, FloatSpan output) const;
	};

	struct AttributeGenerator
	{
		AttributeGeneratorChain chain;
		ConstFloatSpan state;

		void validate() const
		{
			chain.validate();
		}

		void generate(FloatSpan attribute) const
		{
			chain.generate(state, attribute);
		}
	};

	struct AttributeGroupGenerator
	{
		std::vector<AttributeGenerator> generators;

		void validate() const
		{
			for (const AttributeGenerator& generator : generators)
				generator.validate();
		}

		void generate(const std::vector<FloatSpan>& attributes) const;
	};
}
