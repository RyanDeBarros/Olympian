#pragma once

#include "core/types/Polymorphic.h"

namespace oly::particles
{
	namespace internal
	{
		struct Sampler1D;
		struct Domain1D;
		struct Generator1D;
	}

	struct ISampler1D
	{
		virtual void apply(internal::Sampler1D&) const = 0;

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(ISampler1D);
	};

	struct IDomain1D
	{
		virtual void apply(internal::Domain1D&) const = 0;

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(IDomain1D);
	};

	struct AttributeGenerator1D
	{
		Polymorphic<ISampler1D> sampler;
		Polymorphic<IDomain1D> domain;

		void apply(internal::Generator1D& generator) const;
	};
}
