#include "Domains.h"

#include "ShaderStructs.h"

namespace oly::particles
{
	void ConstantDomain1D::apply(internal::Domain1D& domain) const
	{
		domain.type = internal::Domain1D::CONSTANT;
		domain.params[0] = c;
	}

	void LineDomain1D::apply(internal::Domain1D& domain) const
	{
		domain.type = internal::Domain1D::LINE;
		domain.params[0] = a;
		domain.params[1] = b;
	}

	void BiLineDomain1D::apply(internal::Domain1D& domain) const
	{
		domain.type = internal::Domain1D::BILINE;
		domain.params[0] = a;
		domain.params[1] = b;
		domain.params[2] = c;
	}
}
