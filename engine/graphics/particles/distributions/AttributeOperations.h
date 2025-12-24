#pragma once

#include "graphics/particles/AttributeView.h"

namespace oly::particles
{
	struct GenericAttributeOperation1D : public IAttributeOperation1D
	{
		std::function<float(const ParticleEmitter&, float)> fn;

		using IAttributeOperation1D::IAttributeOperation1D;

		float op(const ParticleEmitter& emitter, float attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeOperation1D);
	};

	struct SineAttributeOperation1D : public IAttributeOperation1D
	{
		// attribute = a * sin(b * t - k) + c
		float a = 1.0f;
		float b = 1.0f;
		float k = 0.0f;
		float c = 0.0f;

		using IAttributeOperation1D::IAttributeOperation1D;

		float op(const ParticleEmitter& emitter, float attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SineAttributeOperation1D);
	};

	struct GenericAttributeOperation2D : public IAttributeOperation2D
	{
		std::function<glm::vec2(const ParticleEmitter&, glm::vec2)> fn;

		using IAttributeOperation2D::IAttributeOperation2D;

		glm::vec2 op(const ParticleEmitter& emitter, glm::vec2 attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeOperation2D);
	};

	struct GenericAttributeOperation3D : public IAttributeOperation3D
	{
		std::function<glm::vec3(const ParticleEmitter&, glm::vec3)> fn;

		using IAttributeOperation3D::IAttributeOperation3D;

		glm::vec3 op(const ParticleEmitter& emitter, glm::vec3 attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeOperation3D);
	};

	struct GenericAttributeOperation4D : public IAttributeOperation4D
	{
		std::function<glm::vec4(const ParticleEmitter&, glm::vec4)> fn;

		using IAttributeOperation4D::IAttributeOperation4D;

		glm::vec4 op(const ParticleEmitter& emitter, glm::vec4 attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeOperation4D);
	};
}
