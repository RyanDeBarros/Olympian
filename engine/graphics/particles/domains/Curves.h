#pragma once

#include "graphics/particles/domains/Domain.h"

namespace oly::particles::domains
{
	template<size_t N>
	class Line : public IDomain
	{
	public:
		GLMVector<N> A, B;

		size_t input_dimension() const override
		{
			return 1;
		}
		
		size_t output_dimension() const override
		{
			return N;
		}

		void evaluate(ConstFloatSpan input, FloatSpan output) const override
		{
			output.glm() = A * (1 - input[0]) + B * input[0];
		}
	};
}
