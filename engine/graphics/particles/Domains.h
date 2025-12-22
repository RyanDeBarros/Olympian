#pragma once

#include "graphics/particles/AttributeGenerator.h"

namespace oly::particles
{
	struct ConstantDomain1D : public IDomain1D
	{
		float c;

		ConstantDomain1D(float c = 0.0f) : c(c) {}

		void apply(internal::Domain1D& domain) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(ConstantDomain1D);
	};

	struct LineDomain1D : public IDomain1D
	{
		float a;
		float b;

		LineDomain1D(float a = 0.0f, float b = 0.0f) : a(a), b(b) {}

		void apply(internal::Domain1D& domain) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(LineDomain1D);
	};

	struct BiLineDomain1D : public IDomain1D
	{
		float a;
		float b;
		float c;

		BiLineDomain1D(float a = 0.0f, float b = 0.0f, float c = 0.0f) : a(a), b(b), c(c) {}

		void apply(internal::Domain1D& domain) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(BiLineDomain1D);
	};
}
