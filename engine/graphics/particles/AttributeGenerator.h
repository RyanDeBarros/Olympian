#pragma once

#include "core/types/Polymorphic.h"

namespace oly::particles
{
	namespace internal
	{
		struct Sampler;
		struct Domain;
		struct Generator;
	}

	struct ISampler
	{
		virtual void apply(internal::Sampler&) const = 0;

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(ISampler);
	};

	struct IDomain
	{
		virtual void apply(internal::Domain&) const = 0;

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IDomain);
	};

	struct AttributeGenerator
	{
		Polymorphic<ISampler> sampler;
		Polymorphic<IDomain> domain;

		void apply(internal::Generator& generator) const;
	};
}
