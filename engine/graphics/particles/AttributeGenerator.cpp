#include "AttributeGenerator.h"

#include "core/base/Assert.h"

namespace oly::particles
{
	void AttributeGenerator::validate() const
	{
		OLY_ASSERT(sampler->output_dimension() == domain->input_dimension());
	}

	void AttributeGenerator::generate(ConstFloatSpan state_input, FloatSpan output) const
	{
		// TODO v8 faster rng than rand() / RAND_MAX or <random>
		if (sampler->random_input_dimension() == 0)
			generate(state_input, ConstFloatSpan(nullptr), output);
		else if (sampler->random_input_dimension() == 1)
		{
			float random_input = (float)rand() / RAND_MAX;
			generate(state_input, ConstFloatSpan(random_input), output);
		}
		else
		{
			std::vector<float> random_input(sampler->random_input_dimension());
			for (auto it = random_input.begin(); it != random_input.end(); ++it)
				*it = (float)rand() / RAND_MAX;
			generate(state_input, ConstFloatSpan(random_input), output);
		}
	}

	void AttributeGenerator::generate(ConstFloatSpan state_input, ConstFloatSpan random_input, FloatSpan output) const
	{
		if (domain->input_dimension() == 1)
		{
			float domain_input = 0.0f;
			sampler->sample(state_input, random_input, FloatSpan(domain_input));
			domain->evaluate(ConstFloatSpan(domain_input), output);
		}
		else
		{
			std::vector<float> domain_input(domain->input_dimension());
			sampler->sample(state_input, random_input, FloatSpan(domain_input));
			domain->evaluate(ConstFloatSpan(domain_input), output);
		}
	}
}
