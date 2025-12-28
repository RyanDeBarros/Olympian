#pragma once

#include "core/types/Polymorphic.h"

#include <unordered_set>

namespace oly::particles
{
	struct ParticleEmitter;

	template<typename T>
	struct IAttributeOperation
	{
		virtual ~IAttributeOperation() = default;
		virtual void op(const ParticleEmitter& emitter, T& attribute) const {}

		OLY_POLYMORPHIC_CLONE_DEFINITION(IAttributeOperation<T>);
	};

	template<typename T>
	struct Attribute
	{
		T value;
		Polymorphic<IAttributeOperation<T>> op;

		void on_tick(const ParticleEmitter& emitter) { op->op(emitter, value); }

		auto operator[](size_t i) const
		{
			return value[i];
		}

		auto operator[](size_t i)
		{
			return value[i];
		}

		Attribute<T>& operator=(const T& value)
		{
			this->value = value;
			return *this;
		}

		Attribute<T>& operator=(T&& value)
		{
			this->value = std::move(value);
			return *this;
		}

		operator T() const { return value; }
		operator T& () { return value; }
	};
}
