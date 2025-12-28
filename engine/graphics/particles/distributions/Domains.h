#pragma once

#include "graphics/particles/AttributeGenerator.h"
#include "graphics/particles/Attribute.h"
#include "external/GLM.h"
#include "external/GL.h"

namespace oly::particles
{
	struct ConstantDomain1D : public IDomain1D
	{
		Attribute<float> c;

		ConstantDomain1D(float c = 0.0f) : c(c) {}

		void apply(internal::Domain1D& domain) const override;

		void on_tick(const ParticleEmitter& emitter)
		{
			c.on_tick(emitter);
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(ConstantDomain1D);
	};

	struct LineDomain1D : public IDomain1D
	{
		Attribute<float> a;
		Attribute<float> b;

		LineDomain1D(float a = 0.0f, float b = 0.0f) : a(a), b(b) {}

		void apply(internal::Domain1D& domain) const override;

		void on_tick(const ParticleEmitter& emitter)
		{
			a.on_tick(emitter);
			b.on_tick(emitter);
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(LineDomain1D);
	};

	struct BiLineDomain1D : public IDomain1D
	{
		Attribute<float> a;
		Attribute<float> b;
		Attribute<float> c;

		BiLineDomain1D(float a = 0.0f, float b = 0.0f, float c = 0.0f) : a(a), b(b), c(c) {}

		void apply(internal::Domain1D& domain) const override;

		void on_tick(const ParticleEmitter& emitter)
		{
			a.on_tick(emitter);
			b.on_tick(emitter);
			c.on_tick(emitter);
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(BiLineDomain1D);
	};

	struct ConstantDomain2D : public IDomain2D
	{
		Attribute<glm::vec2> c;

		ConstantDomain2D(glm::vec2 c = {}) : c(c) {}

		void apply(internal::Domain2D& domain) const override;

		void on_tick(const ParticleEmitter& emitter)
		{
			c.on_tick(emitter);
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(ConstantDomain2D);
	};

	struct ConstantDomain3D : public IDomain3D
	{
		Attribute<glm::vec3> c;

		ConstantDomain3D(glm::vec3 c = {}) : c(c) {}

		void apply(internal::Domain3D& domain) const override;

		void on_tick(const ParticleEmitter& emitter)
		{
			c.on_tick(emitter);
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(ConstantDomain3D);
	};

	struct ConstantDomain4D : public IDomain4D
	{
		Attribute<glm::vec4> c;

		ConstantDomain4D(glm::vec4 c = {}) : c(c) {}

		void apply(internal::Domain4D& domain) const override;

		void on_tick(const ParticleEmitter& emitter)
		{
			c.on_tick(emitter);
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(ConstantDomain4D);
	};
}
