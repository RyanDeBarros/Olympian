#pragma once

#include "graphics/particles/FloatSpan.h"

namespace oly::particles::samplers
{
	struct ISampler
	{
		~ISampler() = default;

		virtual size_t state_input_dimension() const = 0;
		virtual size_t random_input_dimension() const = 0;
		virtual size_t output_dimension() const = 0;

		virtual void sample(ConstFloatSpan state_input, ConstFloatSpan random_input, FloatSpan output) const = 0;
	};
}
