#include "Domains.h"

#include "ShaderStructs.h"

namespace oly::particles
{
	void ConstantDomain::apply(internal::Domain& domain) const
	{
		domain.type = internal::Domain::CONSTANT;
		domain.params[0] = c;
	}

	void LineDomain::apply(internal::Domain& domain) const
	{
		domain.type = internal::Domain::LINE;
		domain.params[0] = a;
		domain.params[1] = b;
	}

	void BiLineDomain::apply(internal::Domain& domain) const
	{
		domain.type = internal::Domain::BILINE;
		domain.params[0] = a;
		domain.params[1] = b;
		domain.params[2] = c;
	}
}
