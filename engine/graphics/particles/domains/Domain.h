#pragma once

#include "graphics/particles/FloatSpan.h"

namespace oly::particles::domains
{
	struct IDomain
	{
		~IDomain() = default;

		virtual size_t input_dimension() const = 0;
		virtual size_t output_dimension() const = 0;

		virtual void evaluate(ConstFloatSpan input, FloatSpan output) const = 0;
	};
}
