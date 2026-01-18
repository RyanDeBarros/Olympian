#include "Domains.h"

#include "graphics/particles/ShaderStructs.h"

namespace oly::particles
{
	void ConstantDomain1D::apply(internal::Domain1D& domain) const
	{
		domain.type = internal::Domain1D::Constant;
		domain.params[0] = c;
	}

	void LineDomain1D::apply(internal::Domain1D& domain) const
	{
		domain.type = internal::Domain1D::Line;
		domain.params[0] = a;
		domain.params[1] = b;
	}

	void BiLineDomain1D::apply(internal::Domain1D& domain) const
	{
		domain.type = internal::Domain1D::BiLine;
		domain.params[0] = a;
		domain.params[1] = b;
		domain.params[2] = c;
	}

	void ConstantDomain2D::apply(internal::Domain2D& domain) const
	{
		domain.type = internal::Domain2D::Constant;
		domain.params[0] = c[0];
		domain.params[1] = c[1];
	}

	void ConstantDomain3D::apply(internal::Domain3D& domain) const
	{
		domain.type = internal::Domain3D::Constant;
		domain.params[0] = c[0];
		domain.params[1] = c[1];
		domain.params[2] = c[2];
	}

	void ConstantDomain4D::apply(internal::Domain4D& domain) const
	{
		domain.type = internal::Domain4D::Constant;
		domain.params[0] = c[0];
		domain.params[1] = c[1];
		domain.params[2] = c[2];
		domain.params[3] = c[3];
	}
}
