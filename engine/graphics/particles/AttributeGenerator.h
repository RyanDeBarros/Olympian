#pragma once

#include "graphics/particles/samplers/Sampler.h"
#include "graphics/particles/domains/Domain.h"
#include "core/types/Polymorphic.h"

namespace oly::particles
{
	struct AttributeGeneratorChain
	{
		Polymorphic<samplers::ISampler> sampler;
		Polymorphic<domains::IDomain> domain;

		void validate(ConstFloatSpan attribute) const;

		void generate(ConstFloatSpan state_input, FloatSpan output) const;

	private:
		void generate(ConstFloatSpan state_input, ConstFloatSpan random_input, FloatSpan output) const;
	};

	struct AttributeGenerator
	{
		AttributeGeneratorChain chain;
		ConstFloatSpan state;

		void validate(ConstFloatSpan attribute) const
		{
			chain.validate(attribute);
		}

		void generate(FloatSpan attribute) const
		{
			chain.generate(state, attribute);
		}
	};

	struct AttributeGroupGenerator
	{
		std::vector<AttributeGenerator> generators;

		void validate(ConstFloatSpan attribute) const
		{
			for (const AttributeGenerator& generator : generators)
				generator.validate(attribute);
		}

		void generate(const std::vector<FloatSpan>& attributes) const;
	};
}
