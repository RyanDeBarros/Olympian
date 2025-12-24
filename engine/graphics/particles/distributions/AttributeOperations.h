#pragma once

#include "graphics/particles/AttributeView.h"

namespace oly::particles
{
	template<typename T, size_t N>
	struct SequentialAttributeOperation : public IAttributeOperation<T>
	{
		std::array<Polymorphic<IAttributeOperation<T>>, N> ops;

		void op(const ParticleEmitter& emitter, T& attribute) const override
		{
			for (size_t i = 0; i < N; ++i)
				ops[i]->op(emitter, attribute);
		}

	private:
		using Class = SequentialAttributeOperation<T, N>;

	public:
		OLY_POLYMORPHIC_CLONE_OVERRIDE(Class);
	};

	// TODO v6 more combinatorial operations for things like swizzling, separating vec2 -> individual floats, etc.

	template<typename T>
	struct GenericAttributeOperation : public IAttributeOperation<T>
	{
		std::function<void(const ParticleEmitter&, T&)> fn;

		void op(const ParticleEmitter& emitter, T& attribute) const override
		{
			fn(emitter, attribute);
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericAttributeOperation<T>);
	};

	struct SineAttributeOperation1D : public IAttributeOperation<float>
	{
		// attribute = a * sin(b * t - k) + c
		float a = 1.0f;
		float b = 1.0f;
		float k = 0.0f;
		float c = 0.0f;

		void op(const ParticleEmitter& emitter, float& attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SineAttributeOperation1D);
	};
}
