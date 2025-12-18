#pragma once

#include "graphics/particles/samplers/Sampler.h"
#include "graphics/particles/domains/Domain.h"
// TODO v6 Polymorphic.h should live in core/types, not core/containers.
#include "core/containers/Polymorphic.h"

namespace oly::particles
{
	struct AttributeGenerator
	{
		Polymorphic<samplers::ISampler> sampler;
		Polymorphic<domains::IDomain> domain;

		void validate() const;

		void generate(ConstFloatSpan state_input, FloatSpan output) const;

	private:
		void generate(ConstFloatSpan state_input, ConstFloatSpan random_input, FloatSpan output) const;
	};
}
