#pragma once

#include "graphics/particles/Attribute.h"
#include "external/GLM.h"

#include <functional>

namespace oly::particles::operations
{
	template<typename T, size_t N>
	struct Sequence : public IAttributeOperation<T>
	{
		std::array<Polymorphic<IAttributeOperation<T>>, N> ops;

		Sequence() = default;

		template<typename... Ops> requires (sizeof...(Ops) == N)
		explicit Sequence(Ops&&... operations)
			: ops{ std::forward<Ops>(operations)... }
		{
		}

		void op(const ParticleEmitter& emitter, T& attribute) const override
		{
			for (size_t i = 0; i < N; ++i)
				ops[i]->op(emitter, attribute);
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(Sequence<T, N>);
	};

	template<typename T, typename U>
	struct Selector : public IAttributeOperation<T>
	{
		Polymorphic<IAttributeOperation<U>> inner_op;

		using Selection = U& (*)(T&);
		Selection selection = nullptr;

		Selector() = default;
		Selector(const Polymorphic<IAttributeOperation<U>>& inner_op, Selection selection) : inner_op(inner_op), selection(selection) {}
		Selector(Polymorphic<IAttributeOperation<U>>&& inner_op, Selection selection) : inner_op(std::move(inner_op)), selection(selection) {}

		void op(const ParticleEmitter& emitter, T& attribute) const override
		{
			inner_op->op(emitter, selection(attribute));
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(Selector<T, U>);
	};

	template<typename T>
	struct GenericFunction : public IAttributeOperation<T>
	{
		using Function = std::function<void(const ParticleEmitter&, T&)>;
		Function fn;

		GenericFunction() = default;
		GenericFunction(const Function& fn) : fn(fn) {}
		GenericFunction(Function&& fn) : fn(std::move(fn)) {}

		void op(const ParticleEmitter& emitter, T& attribute) const override
		{
			fn(emitter, attribute);
		}

		OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericFunction<T>);
	};

	struct SineWave1D : public IAttributeOperation<float>
	{
		// attribute = a * sin(b * t - k) + c
		float a = 1.0f;
		float b = 1.0f;
		float k = 0.0f;
		float c = 0.0f;

		SineWave1D() = default;
		SineWave1D(float a, float b, float k, float c) : a(a), b(b), k(k), c(c) {}

		void op(const ParticleEmitter& emitter, float& attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(SineWave1D);
	};

	struct Polarization2D : public IAttributeOperation<glm::vec2>
	{
		float amplitude = 1.0f;
		float time_offset = 0.0f;

		Polarization2D() = default;
		Polarization2D(float amplitude, float time_offset) : amplitude(amplitude), time_offset(time_offset) {}

		void op(const ParticleEmitter& emitter, glm::vec2& attribute) const override;

		OLY_POLYMORPHIC_CLONE_OVERRIDE(Polarization2D);
	};
}
