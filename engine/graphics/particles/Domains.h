#pragma once

#include "graphics/particles/AttributeGenerator.h"

namespace oly::particles
{
	struct ConstantDomain : public IDomain
	{
		float c;

		ConstantDomain(float c = 0.0f) : c(c) {}

		void apply(internal::Domain& domain) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(ConstantDomain);
	};

	struct LineDomain : public IDomain
	{
		float a;
		float b;

		LineDomain(float a = 0.0f, float b = 0.0f) : a(a), b(b) {}

		void apply(internal::Domain& domain) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(LineDomain);
	};

	struct BiLineDomain : public IDomain
	{
		float a;
		float b;
		float c;

		BiLineDomain(float a = 0.0f, float b = 0.0f, float c = 0.0f) : a(a), b(b), c(c) {}

		void apply(internal::Domain& domain) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(BiLineDomain);
	};
}
