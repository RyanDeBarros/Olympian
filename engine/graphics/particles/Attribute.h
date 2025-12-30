#pragma once

#include "core/types/Polymorphic.h"
#include "core/util/Loader.h"
#include "core/util/Enum.h"

#include <unordered_set>
#include <functional>
#include <array>

namespace oly::particles
{
	struct ParticleEmitter;

#define _SubSelectorEntryMap(T)\
			T(NONE, 0)\
			T(X, 1)\
			T(R, 1)\
			T(Y, 2)\
			T(G, 2)\
			T(Z, 3)\
			T(B, 3)\
			T(W, 4)\
			T(A, 4)\
			T(XY, 5)\
			T(RG, 5)\
			T(XZ, 6)\
			T(RB, 6)\
			T(XW, 7)\
			T(RA, 7)\
			T(YZ, 8)\
			T(GB, 8)\
			T(YW, 9)\
			T(GA, 9)\
			T(ZW, 10)\
			T(BA, 10)\
			T(XYZ, 11)\
			T(RGB, 11)\
			T(XYW, 12)\
			T(RGA, 12)\
			T(XZW, 13)\
			T(RBA, 13)\
			T(YZW, 14)\
			T(GBA, 14)
	OLY_ENUM(SubSelector, _SubSelectorEntryMap);
#undef _SubSelectorEntryMap

	template<bool Const>
	class TAttributeSpan
	{
		using PointerType = std::conditional_t<Const, const float*, float*>;
		template<typename T>
		using ArgumentType = std::conditional_t<Const, T, T&>;

		// TODO v6 use float* base = nullptr; with std::array<uint8_t, 4> offsets instead.
		std::array<PointerType, 4> refs = { nullptr, nullptr, nullptr, nullptr };
		uint8_t size = 0;

	public:
		TAttributeSpan() {}
		explicit TAttributeSpan(ArgumentType<float> v) : refs({ &v, nullptr, nullptr, nullptr }), size(1) {}
		explicit TAttributeSpan(ArgumentType<glm::vec2> v) : refs({ &v.x, &v.y, nullptr, nullptr }), size(2) {}
		explicit TAttributeSpan(ArgumentType<glm::vec3> v) : refs({ &v.x, &v.y, &v.z, nullptr }), size(3) {}
		explicit TAttributeSpan(ArgumentType<glm::vec4> v) : refs({ &v.x, &v.y, &v.z, &v.w }), size(4) {}

		template<bool OtherConst>
		TAttributeSpan(TAttributeSpan<OtherConst> span) requires (Const || !OtherConst) : refs(span.refs), size(span.size) {}

		template<bool OtherConst = Const>
		TAttributeSpan<OtherConst> select(SubSelector sel) requires (!Const || OtherConst)
		{
			TAttributeSpan span;
			switch (sel)
			{
			case SubSelector::NONE:
				for (size_t i = 0; i < size; ++i)
					span.refs[i] = refs[i];
				span.size = size;
				break;
			case SubSelector::X:
				span.refs[0] = refs[0];
				span.size = 1;
				break;
			case SubSelector::Y:
				span.refs[0] = refs[1];
				span.size = 1;
				break;
			case SubSelector::Z:
				span.refs[0] = refs[2];
				span.size = 1;
				break;
			case SubSelector::W:
				span.refs[0] = refs[3];
				span.size = 1;
				break;
			case SubSelector::XY:
				span.refs[0] = refs[0];
				span.refs[1] = refs[1];
				span.size = 2;
				break;
			case SubSelector::XZ:
				span.refs[0] = refs[0];
				span.refs[1] = refs[2];
				span.size = 2;
				break;
			case SubSelector::XW:
				span.refs[0] = refs[0];
				span.refs[1] = refs[3];
				span.size = 2;
				break;
			case SubSelector::YZ:
				span.refs[0] = refs[1];
				span.refs[1] = refs[2];
				span.size = 2;
				break;
			case SubSelector::YW:
				span.refs[0] = refs[1];
				span.refs[1] = refs[3];
				span.size = 2;
				break;
			case SubSelector::ZW:
				span.refs[0] = refs[2];
				span.refs[1] = refs[3];
				span.size = 2;
				break;
			case SubSelector::XYZ:
				span.refs[0] = refs[0];
				span.refs[1] = refs[1];
				span.refs[2] = refs[2];
				span.size = 3;
				break;
			case SubSelector::XYW:
				span.refs[0] = refs[0];
				span.refs[1] = refs[1];
				span.refs[2] = refs[3];
				span.size = 3;
				break;
			case SubSelector::XZW:
				span.refs[0] = refs[0];
				span.refs[1] = refs[2];
				span.refs[2] = refs[3];
				span.size = 3;
				break;
			case SubSelector::YZW:
				span.refs[0] = refs[1];
				span.refs[1] = refs[2];
				span.refs[2] = refs[3];
				span.size = 3;
				break;
			default:
				throw Error(ErrorCode::UNSUPPORTED_SWITCH_CASE);
			}
			return span;
		}

		float& operator[](size_t i) requires (!Const)
		{
			if (i < size) [[likely]]
				return *refs[i];
			else
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}

		float operator[](size_t i) const
		{
			if (i < size) [[likely]]
				return *refs[i];
			else
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
		}

		uint8_t length() const
		{
			return size;
		}

		TAttributeSpan& operator=(float v) requires (!Const)
		{
			if (size == 1)
			{
				*refs[0] = v;
				return *this;
			}
			else
				throw Error(ErrorCode::INVALID_SIZE);
		}

		TAttributeSpan& operator=(glm::vec2 v) requires (!Const)
		{
			if (size == 2)
			{
				*refs[0] = v.x;
				*refs[1] = v.y;
				return *this;
			}
			else
				throw Error(ErrorCode::INVALID_SIZE);
		}

		TAttributeSpan& operator=(glm::vec3 v) requires (!Const)
		{
			if (size == 3)
			{
				*refs[0] = v.x;
				*refs[1] = v.y;
				*refs[2] = v.z;
				return *this;
			}
			else
				throw Error(ErrorCode::INVALID_SIZE);
		}

		TAttributeSpan& operator=(glm::vec4 v) requires (!Const)
		{
			if (size == 4)
			{
				*refs[0] = v.x;
				*refs[1] = v.y;
				*refs[2] = v.z;
				*refs[3] = v.w;
				return *this;
			}
			else
				throw Error(ErrorCode::INVALID_SIZE);
		}

		float flt() const
		{
			if (size == 1)
				return *refs[0];
			else
				throw Error(ErrorCode::INVALID_SIZE);
		}

		glm::vec2 vec2() const
		{
			if (size == 2)
				return { *refs[0], *refs[1] };
			else
				throw Error(ErrorCode::INVALID_SIZE);
		}

		glm::vec3 vec3() const
		{
			if (size == 3)
				return { *refs[0], *refs[1], *refs[2] };
			else
				throw Error(ErrorCode::INVALID_SIZE);
		}

		glm::vec4 vec4() const
		{
			if (size == 4)
				return { *refs[0], *refs[1], *refs[2], *refs[3] };
			else
				throw Error(ErrorCode::INVALID_SIZE);
		}
	};

	using AttributeSpan = TAttributeSpan<false>;
	using ConstAttributeSpan = TAttributeSpan<true>;

	struct IAttributeOperation
	{
		virtual ~IAttributeOperation() = default;
		virtual void op(const ParticleEmitter& emitter, AttributeSpan attribute) const {}

		OLY_POLYMORPHIC_CLONE_DEFINITION(IAttributeOperation);

		static Polymorphic<IAttributeOperation> load(TOMLNode node);
	};

	template<typename T>
	struct Attribute
	{
		T value;
		Polymorphic<IAttributeOperation> op;

		void on_tick(const ParticleEmitter& emitter) { if (op) op->op(emitter, AttributeSpan(value)); }

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

	namespace operations
	{
		template<size_t N>
		struct Sequence : public IAttributeOperation
		{
			std::array<Polymorphic<IAttributeOperation>, N> ops;

			Sequence() = default;

			template<typename... Ops> requires (sizeof...(Ops) == N)
			explicit Sequence(Ops&&... operations)
				: ops{ std::forward<Ops>(operations)... }
			{
			}

			explicit Sequence(std::array<Polymorphic<IAttributeOperation>, N>&& operations)
				: ops(std::move(operations))
			{
			}

			void op(const ParticleEmitter& emitter, AttributeSpan attribute) const override
			{
				for (size_t i = 0; i < N; ++i)
					ops[i]->op(emitter, attribute);
			}

			OLY_POLYMORPHIC_CLONE_OVERRIDE(Sequence<N>);
		};

		template<>
		struct Sequence<0> : public IAttributeOperation
		{
			std::vector<Polymorphic<IAttributeOperation>> ops;

			Sequence() = default;

			template<typename... Ops>
			explicit Sequence(Ops&&... operations)
				: ops{ std::forward<Ops>(operations)... }
			{
			}

			explicit Sequence(std::vector<Polymorphic<IAttributeOperation>>&& operations)
				: ops(std::move(operations))
			{
			}

			void op(const ParticleEmitter& emitter, AttributeSpan attribute) const override
			{
				for (const auto& op : ops)
					op->op(emitter, attribute);
			}

			static Polymorphic<Sequence<0>> load(TOMLNode node);
			static Polymorphic<IAttributeOperation> load_fixed(TOMLNode node);

			OLY_POLYMORPHIC_CLONE_OVERRIDE(Sequence<0>);
		};

		struct Selector : public IAttributeOperation
		{
			Polymorphic<IAttributeOperation> inner_op;

			SubSelector selector = SubSelector::NONE;

			Selector() = default;
			Selector(const Polymorphic<IAttributeOperation>& inner_op, SubSelector selector) : inner_op(inner_op), selector(selector) {}
			Selector(Polymorphic<IAttributeOperation>&& inner_op, SubSelector selector) : inner_op(std::move(inner_op)), selector(selector) {}

			void op(const ParticleEmitter& emitter, AttributeSpan attribute) const override
			{
				//if (inner_op)
					inner_op->op(emitter, attribute.select(selector));
			}

			static Polymorphic<Selector> load(TOMLNode node);

			OLY_POLYMORPHIC_CLONE_OVERRIDE(Selector);
		};

		struct GenericFunction : public IAttributeOperation
		{
			using Function = std::function<void(const ParticleEmitter&, AttributeSpan)>;
			Function fn;

			GenericFunction() = default;
			GenericFunction(const Function& fn) : fn(fn) {}
			GenericFunction(Function&& fn) : fn(std::move(fn)) {}

			void op(const ParticleEmitter& emitter, AttributeSpan attribute) const override
			{
				fn(emitter, attribute);
			}

			OLY_POLYMORPHIC_CLONE_OVERRIDE(GenericFunction);
		};
	}
}
