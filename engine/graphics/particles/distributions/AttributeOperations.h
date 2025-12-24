#pragma once

#include "graphics/particles/AttributeView.h"

namespace oly::particles
{
	struct GenericAttributeOperation1D : public IAttributeOperation1D
	{
		std::function<float(const ParticleEmitter&, float)> fn;

		float op(const ParticleEmitter& emitter, float attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeOperation1D);
	};

	template<size_t N>
	struct SequentialAttributeOperation1D : public IAttributeOperation1D
	{
		std::array<Polymorphic<IAttributeOperation1D>, N> ops;

		float op(const ParticleEmitter& emitter, float attribute) const override
		{
			for (size_t i = 0; i < N; ++i)
				attribute = ops[i]->op(emitter, attribute);
			return attribute;
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SequentialAttributeOperation1D<N>);
	};

	struct SineAttributeOperation1D : public IAttributeOperation1D
	{
		// attribute = a * sin(b * t - k) + c
		float a = 1.0f;
		float b = 1.0f;
		float k = 0.0f;
		float c = 0.0f;

		float op(const ParticleEmitter& emitter, float attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SineAttributeOperation1D);
	};

	struct GenericAttributeOperation2D : public IAttributeOperation2D
	{
		std::function<glm::vec2(const ParticleEmitter&, glm::vec2)> fn;

		glm::vec2 op(const ParticleEmitter& emitter, glm::vec2 attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeOperation2D);
	};

	template<size_t N>
	struct SequentialAttributeOperation2D : public IAttributeOperation2D
	{
		std::array<Polymorphic<IAttributeOperation2D>, N> ops;

		glm::vec2 op(const ParticleEmitter& emitter, glm::vec2 attribute) const override
		{
			for (size_t i = 0; i < N; ++i)
				attribute = ops[i]->op(emitter, attribute);
			return attribute;
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SequentialAttributeOperation2D<N>);
	};

	struct GenericAttributeOperation3D : public IAttributeOperation3D
	{
		std::function<glm::vec3(const ParticleEmitter&, glm::vec3)> fn;

		glm::vec3 op(const ParticleEmitter& emitter, glm::vec3 attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeOperation3D);
	};

	template<size_t N>
	struct SequentialAttributeOperation3D : public IAttributeOperation3D
	{
		std::array<Polymorphic<IAttributeOperation3D>, N> ops;

		glm::vec3 op(const ParticleEmitter& emitter, glm::vec3 attribute) const override
		{
			for (size_t i = 0; i < N; ++i)
				attribute = ops[i]->op(emitter, attribute);
			return attribute;
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SequentialAttributeOperation3D<N>);
	};

	struct GenericAttributeOperation4D : public IAttributeOperation4D
	{
		std::function<glm::vec4(const ParticleEmitter&, glm::vec4)> fn;

		glm::vec4 op(const ParticleEmitter& emitter, glm::vec4 attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeOperation4D);
	};

	template<size_t N>
	struct SequentialAttributeOperation4D : public IAttributeOperation4D
	{
		std::array<Polymorphic<IAttributeOperation4D>, N> ops;

		glm::vec4 op(const ParticleEmitter& emitter, glm::vec4 attribute) const override
		{
			for (size_t i = 0; i < N; ++i)
				attribute = ops[i]->op(emitter, attribute);
			return attribute;
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SequentialAttributeOperation4D<N>);
	};
}
