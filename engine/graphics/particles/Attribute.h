#pragma once

#include "core/types/Polymorphic.h"
#include "core/util/Loader.h"

#include <unordered_set>
#include <functional>

namespace oly::particles
{
	struct ParticleEmitter;

	template<typename T>
	struct IAttributeOperation
	{
		virtual ~IAttributeOperation() = default;
		virtual void op(const ParticleEmitter& emitter, T& attribute) const {}

		OLY_POLYMORPHIC_CLONE_DEFINITION(IAttributeOperation<T>);

		static Polymorphic<IAttributeOperation> load(TOMLNode node);
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
		T operator*() const { return value; }
		T& operator*() { return value; }
		const T* operator->() const { return &value; }
		T* operator->() { return &value; }
	};

	struct Enum
	{
		unsigned int value;

		virtual ~Enum() = default;
		virtual const std::unordered_map<std::string, unsigned int>& names() const = 0;

		void load_string(const std::string& name)
		{
			auto it = names().find(name);
			if (it != names().end())
				value = it->second;
		}

		OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(Enum);
	};

	namespace operations
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

			using Selection = U & (*)(T&);
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
	}
}
