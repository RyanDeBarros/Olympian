#pragma once

#include "graphics/particles/AttributeGenerator.h"
#include "graphics/particles/Attribute.h"
#include "external/GLM.h"
#include "external/GL.h"

namespace oly::particles
{
	struct ConstantDomain1D : public IDomain1D
	{
        IMP_POLYMORPHIC_IMPL((ConstantDomain1D));

		Attribute<float> c;

		ConstantDomain1D(float c = 0.0f) : c(c) {}

		void apply(internal::Domain1D& domain) const override;

		void on_tick(const ParticleEmitter& emitter) override
		{
			c.on_tick(emitter);
		}

		void overload(TOMLNode node) override;
	};

	struct LineDomain1D : public IDomain1D
	{
        IMP_POLYMORPHIC_IMPL((LineDomain1D));

		Attribute<float> a;
		Attribute<float> b;

		LineDomain1D(float a = 0.0f, float b = 0.0f) : a(a), b(b) {}

		void apply(internal::Domain1D& domain) const override;

		void on_tick(const ParticleEmitter& emitter) override
		{
			a.on_tick(emitter);
			b.on_tick(emitter);
		}

		void overload(TOMLNode node) override;
	};

	struct BiLineDomain1D : public IDomain1D
	{
        IMP_POLYMORPHIC_IMPL((BiLineDomain1D));

		Attribute<float> a;
		Attribute<float> b;
		Attribute<float> c;

		BiLineDomain1D(float a = 0.0f, float b = 0.0f, float c = 0.0f) : a(a), b(b), c(c) {}

		void apply(internal::Domain1D& domain) const override;

		void on_tick(const ParticleEmitter& emitter) override
		{
			a.on_tick(emitter);
			b.on_tick(emitter);
			c.on_tick(emitter);
		}

		void overload(TOMLNode node) override;
	};

	struct ConstantDomain2D : public IDomain2D
	{
        IMP_POLYMORPHIC_IMPL((ConstantDomain2D));

		Attribute<glm::vec2> c;

		ConstantDomain2D(glm::vec2 c = {}) : c(c) {}

		void apply(internal::Domain2D& domain) const override;

		void on_tick(const ParticleEmitter& emitter) override
		{
			c.on_tick(emitter);
		}

		void overload(TOMLNode node) override;
	};

	struct ConstantDomain3D : public IDomain3D
	{
        IMP_POLYMORPHIC_IMPL((ConstantDomain3D));

		Attribute<glm::vec3> c;

		ConstantDomain3D(glm::vec3 c = {}) : c(c) {}

		void apply(internal::Domain3D& domain) const override;

		void on_tick(const ParticleEmitter& emitter) override
		{
			c.on_tick(emitter);
		}

		void overload(TOMLNode node) override;
	};

	struct ConstantDomain4D : public IDomain4D
	{
        IMP_POLYMORPHIC_IMPL((ConstantDomain4D));

		Attribute<glm::vec4> c;

		ConstantDomain4D(glm::vec4 c = {}) : c(c) {}

		void apply(internal::Domain4D& domain) const override;

		void on_tick(const ParticleEmitter& emitter) override
		{
			c.on_tick(emitter);
		};

		void overload(TOMLNode node) override;
	};
}
