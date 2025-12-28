#pragma once

#include "graphics/particles/Attribute.h"
#include "external/GLM.h"

#include <functional>

namespace oly::particles
{
	template<typename T, size_t N>
	struct SequentialAttributeOperation : public IAttributeOperation<T>
	{
		std::array<Polymorphic<IAttributeOperation<T>>, N> ops;

		SequentialAttributeOperation() = default;

		template<typename... Ops> requires (sizeof...(Ops) == N)
		explicit SequentialAttributeOperation(Ops&&... operations)
			: ops{ std::forward<Ops>(operations)... }
		{
		}

		void op(const ParticleEmitter& emitter, T& attribute) const override
		{
			for (size_t i = 0; i < N; ++i)
				ops[i]->op(emitter, attribute);
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SequentialAttributeOperation<T, N>);
	};

	template<typename T, typename U>
	struct SelectorAttributeOperation : public IAttributeOperation<T>
	{
		Polymorphic<IAttributeOperation<U>> inner_op;

		using Selector = U& (*)(T&);
		Selector selector = nullptr;

		SelectorAttributeOperation() = default;
		SelectorAttributeOperation(const Polymorphic<IAttributeOperation<U>>& inner_op, Selector selector) : inner_op(inner_op), selector(selector) {}
		SelectorAttributeOperation(Polymorphic<IAttributeOperation<U>>&& inner_op, Selector selector) : inner_op(std::move(inner_op)), selector(selector) {}

		void op(const ParticleEmitter& emitter, T& attribute) const override
		{
			inner_op->op(emitter, selector(attribute));
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SelectorAttributeOperation<T, U>);
	};

	template<typename T>
	struct GenericAttributeOperation : public IAttributeOperation<T>
	{
		using Function = std::function<void(const ParticleEmitter&, T&)>;
		Function fn;

		GenericAttributeOperation() = default;
		GenericAttributeOperation(const Function& fn) : fn(fn) {}
		GenericAttributeOperation(Function&& fn) : fn(std::move(fn)) {}

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

		SineAttributeOperation1D() = default;
		SineAttributeOperation1D(float a, float b, float k, float c) : a(a), b(b), k(k), c(c) {}

		void op(const ParticleEmitter& emitter, float& attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SineAttributeOperation1D);
	};
}
