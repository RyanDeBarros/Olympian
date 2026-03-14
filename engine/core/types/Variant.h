#pragma once

#include "core/types/DeferredFalse.h"
#include "core/base/Errors.h"
#include "core/util/Macros.h"

namespace oly
{
	template<typename... Types>
	class Variant
	{
		static_assert(deferred_false<Variant<Types...>>);
	};

#define _OLY_ENUM_DEF(N) _##N
#define _OLY_TYPE_ENUM(N)\
	public:\
	enum class Type\
	{\
		NONE,\
		_OLY_REPEAT_COMMA(_OLY_ENUM_DEF, N)\
	};\
	private:\
		Type type;

#define _OLY_STORAGE_DEF(N) T##N t##N;
#define _OLY_STORAGE(N) union Storage\
	{\
		_OLY_REPEAT(_OLY_STORAGE_DEF, N)\
		Storage() {}\
		~Storage() {}\
	} storage;

#define _OLY_DEL_CASE(N) case Type::_##N: storage.t##N.~T##N(); break;
#define _OLY_DEL(N)\
	void del()\
	{\
		switch (type)\
		{\
			_OLY_REPEAT(_OLY_DEL_CASE, N);\
		default: break;\
		}\
		type = Type::NONE;\
	}

#define _OLY_COPY_CASE(N) case Type::_##N: new (&storage.t##N) T##N(other.storage.t##N); break;
#define _OLY_COPY_FROM(N)\
	void copy_from(const Variant& other)\
	{\
		type = other.type;\
		switch (type)\
		{\
			_OLY_REPEAT(_OLY_COPY_CASE, N);\
		default: break;\
		}\
	}

#define _OLY_MOVE_CASE(N) case Type::_##N: new (&storage.t##N) T##N(std::move(other.storage.t##N)); break;
#define _OLY_MOVE_FROM(N)\
	void move_from(Variant&& other) noexcept\
	{\
		type = other.type;\
		switch (type)\
		{\
			_OLY_REPEAT(_OLY_MOVE_CASE, N);\
		default: break;\
		}\
		other.type = Type::NONE;\
	}

#define _OLY_PTR_CASE(N) case Type::_##N: return &storage.t##N;
#define _OLY_PTR(N)\
	const void* ptr() const\
	{\
		switch (type)\
		{\
			_OLY_REPEAT(_OLY_PTR_CASE, N);\
		default: return nullptr;\
		}\
	}\
	void* ptr()\
	{\
		switch (type)\
		{\
			_OLY_REPEAT(_OLY_PTR_CASE, N);\
		default: return nullptr;\
		}\
	}

#define _OLY_VALUE_CTOR(N)\
	Variant(const T##N& t##N)\
		: type(Type::_##N)\
	{\
		new (&storage.t##N) T##N(t##N);\
	}\
	Variant(T##N&& t##N) noexcept\
		: type(Type::_##N)\
	{\
		new (&storage.t##N) T##N(std::move(t##N));\
	}\
	Variant& operator=(const T##N& t##N)\
	{\
		if (type == Type::_##N)\
			storage.t##N = t##N;\
		else\
		{\
			del();\
			type = Type::_##N;\
			new (&storage.t##N) T##N(t##N);\
		}\
		return *this;\
	}\
	Variant& operator=(T##N&& t##N) noexcept\
	{\
		if (type == Type::_##N)\
			storage.t##N = std::move(t##N);\
		else\
		{\
			del();\
			type = Type::_##N;\
			new (&storage.t##N) T##N(std::move(t##N));\
		}\
		return *this;\
	}

#define _OLY_CTOR(N)\
	Variant() : type(Type::NONE) {}\
	_OLY_REPEAT(_OLY_VALUE_CTOR, N)\
	Variant(const Variant & other) { copy_from(other); }\
	Variant(Variant && other) noexcept { move_from(std::move(other)); }\
	~Variant() { del(); }\
	Variant& operator=(const Variant & other) { if (this != &other) { del(); copy_from(other); } return *this; }\
	Variant& operator=(Variant && other) noexcept { if (this != &other) { del(); move_from(std::move(other)); } return *this; }

#define _OLY_HOLDS_CHECK(N) if constexpr (std::is_same_v<T, T##N>) return type == Type::_##N;
#define _OLY_HOLDS(N)\
	template<typename T>\
	bool holds() const\
	{\
		_OLY_REPEAT(_OLY_HOLDS_CHECK, N);\
		return false;\
	}

#define _OLY_GET(N)\
	template<typename T>\
	const T& get() const\
	{\
		if (holds<T>())\
			return *reinterpret_cast<const T*>(ptr());\
		else\
			throw Error(ErrorCode::BadVariant);\
	}\
	template<typename T>\
	T& get()\
	{\
		if (holds<T>())\
			return *reinterpret_cast<T*>(ptr());\
		else\
			throw Error(ErrorCode::BadVariant);\
	}\
	template<typename T>\
	const T* safe_get() const\
	{\
		if (holds<T>())\
			return reinterpret_cast<const T*>(ptr());\
		else\
			return nullptr;\
	}\
	template<typename T>\
	T* safe_get()\
	{\
		if (holds<T>())\
			return reinterpret_cast<T*>(ptr());\
		else\
			return nullptr;\
	}

#define _OLY_INVOKE_CASE(N) case Type::_##N: return std::invoke(std::forward<F##N>(f##N), storage.t##N);
#define _OLY_VISIT_Single_CASE(N) case Type::_##N: return std::invoke(std::forward<F>(f), storage.t##N);
#define _OLY_TYPENAME_F(N) typename F##N
#define _OLY_VISIT_ARG(N) F##N&& f##N
#define _OLY_VISIT(N)\
	template<_OLY_REPEAT_COMMA(_OLY_TYPENAME_F, N)>\
	decltype(auto) visit(_OLY_REPEAT_COMMA(_OLY_VISIT_ARG, N)) const\
	{\
		switch (type)\
		{\
			_OLY_REPEAT(_OLY_INVOKE_CASE, N);\
		default: throw Error(ErrorCode::BadVariant);\
		}\
	}\
	template<_OLY_REPEAT_COMMA(_OLY_TYPENAME_F, N)>\
	decltype(auto) visit(_OLY_REPEAT_COMMA(_OLY_VISIT_ARG, N))\
	{\
		switch (type)\
		{\
			_OLY_REPEAT(_OLY_INVOKE_CASE, N);\
		default: throw Error(ErrorCode::BadVariant);\
		}\
	}\
	template<typename F>\
	decltype(auto) visit(F&& f) const\
	{\
		switch (type)\
		{\
			_OLY_REPEAT(_OLY_VISIT_Single_CASE, N);\
		default: throw Error(ErrorCode::BadVariant);\
		}\
	}\
	template<typename F>\
	decltype(auto) visit(F&& f)\
	{\
		switch (type)\
		{\
			_OLY_REPEAT(_OLY_VISIT_Single_CASE, N);\
		default: throw Error(ErrorCode::BadVariant);\
		}\
	}

#define _OLY_TYPE_OF_CASE(N) if constexpr (std::is_same_v<T, T##N>) return Type::_##N;
#define _OLY_GET_TYPE(N)\
	Type get_type() const { return type; }\
	template<typename T>\
	static constexpr Type type_of()\
	{\
		_OLY_REPEAT(_OLY_TYPE_OF_CASE, N);\
		static_assert(deferred_false<T>);\
	}

#define _OLY_TYPENAME_T(N) typename T##N
#define _OLY_TYPE_PARAMETER(N) T##N
#define _OLY_VARIANT(N)\
	template<_OLY_REPEAT_COMMA(_OLY_TYPENAME_T, N)>\
	class Variant<_OLY_REPEAT_COMMA(_OLY_TYPE_PARAMETER, N)>\
	{\
		_OLY_TYPE_ENUM(N);\
		_OLY_STORAGE(N);\
		_OLY_DEL(N);\
		_OLY_COPY_FROM(N);\
		_OLY_MOVE_FROM(N);\
		_OLY_PTR(N);\
	public:\
		_OLY_CTOR(N);\
		_OLY_HOLDS(N);\
		bool empty() const { return type == Type::NONE; }\
		_OLY_GET(N);\
		_OLY_VISIT(N);\
		_OLY_GET_TYPE(N);\
	};

	// ###########################################################################
	_OLY_VARIANT(2);
	_OLY_VARIANT(3);
	_OLY_VARIANT(4);
	_OLY_VARIANT(5);
	_OLY_VARIANT(6);
	_OLY_VARIANT(7);
	_OLY_VARIANT(8);
	_OLY_VARIANT(9);
	_OLY_VARIANT(10);
	_OLY_VARIANT(11);
	_OLY_VARIANT(12);
	_OLY_VARIANT(13);
	_OLY_VARIANT(14);
	_OLY_VARIANT(15);
	_OLY_VARIANT(16);
	// ###########################################################################

#undef _OLY_ENUM_DEF
#undef _OLY_TYPE_ENUM
#undef _OLY_STORAGE_DEF
#undef _OLY_STORAGE
#undef _OLY_DEL_CASE
#undef _OLY_DEL
#undef _OLY_COPY_CASE
#undef _OLY_COPY_FROM
#undef _OLY_MOVE_CASE
#undef _OLY_MOVE_FROM
#undef _OLY_PTR_CASE
#undef _OLY_PTR
#undef _OLY_VALUE_CTOR
#undef _OLY_CTOR
#undef _OLY_HOLDS_CHECK
#undef _OLY_HOLDS
#undef _OLY_GET
#undef _OLY_INVOKE_CASE
#undef _OLY_TYPENAME_F
#undef _OLY_VISIT_ARG
#undef _OLY_VISIT
#undef _OLY_TYPE_OF_CASE
#undef _OLY_GET_TYPE
#undef _OLY_TYPENAME_T
#undef _OLY_TYPE_PARAMETER
#undef _OLY_VARIANT
}
