#pragma once

#include "graphics/particles/AttributeGenerator.h"
#include "external/GLM.h"
#include "external/GL.h"

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

	struct ConstantDomain2D : public IDomain2D
	{
		glm::vec2 c;

		ConstantDomain2D(glm::vec2 c = {}) : c(c) {}

		void apply(internal::Domain2D& domain) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(ConstantDomain2D);
	};

	struct ConstantDomain3D : public IDomain3D
	{
		glm::vec3 c;

		ConstantDomain3D(glm::vec3 c = {}) : c(c) {}

		void apply(internal::Domain3D& domain) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(ConstantDomain3D);
	};

	struct ConstantDomain4D : public IDomain4D
	{
		glm::vec4 c;

		ConstantDomain4D(glm::vec4 c = {}) : c(c) {}

		void apply(internal::Domain4D& domain) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(ConstantDomain4D);
	};
}
