#pragma once

#include "core/base/Errors.h"
#include "core/types/Meta.h"
#include "core/types/Macros.h"

namespace oly
{
	template<typename T>
	class Polymorphic
	{
		T* _raw = nullptr;

		template<typename>
		friend class Polymorphic;

	public:
		Polymorphic()
			: _raw(new T())
		{
		}

		Polymorphic(T* raw)
			: _raw(raw)
		{
			if (!_raw)
				throw Error(ErrorCode::NULL_POINTER);
		}

		Polymorphic(const Polymorphic& other)
			: _raw(other._raw->clone_new())
		{
		}

		Polymorphic(Polymorphic&& other) noexcept
			: _raw(std::move(*other._raw).clone_new())
		{
		}

		template<PolymorphicBaseOf<T> U>
		Polymorphic(const Polymorphic<U>& other)
		{
			U* temp = other._raw->clone_new();
			_raw = dynamic_cast<T*>(temp);
			if (!_raw)
			{
				delete temp;
				throw Error(ErrorCode::BAD_CAST);
			}
		}

		template<PolymorphicBaseOf<T> U>
		Polymorphic(Polymorphic<U>&& other)
		{
			U* temp = std::move(*other._raw).clone_new();
			_raw = dynamic_cast<T*>(temp);
			if (!_raw)
			{
				delete temp;
				throw Error(ErrorCode::BAD_CAST);
			}
		}

		~Polymorphic()
		{
			delete _raw;
		}

		Polymorphic& operator=(const Polymorphic& other)
		{
			if (this != &other)
			{
				delete _raw;
				_raw = other._raw->clone_new();
			}
			return *this;
		}

		Polymorphic& operator=(Polymorphic&& other) noexcept
		{
			if (this != &other)
			{
				delete _raw;
				_raw = std::move(*other._raw).clone_new();
			}
			return *this;
		}

		template<PolymorphicBaseOf<T> U>
		Polymorphic<T>& operator=(const Polymorphic<U>& other)
		{
			delete _raw;
			U* temp = other._raw->clone_new();
			_raw = dynamic_cast<T*>(temp);
			if (!_raw)
			{
				delete temp;
				throw Error(ErrorCode::BAD_CAST);
			}
			return *this;
		}

		template<PolymorphicBaseOf<T> U>
		Polymorphic<T>& operator=(Polymorphic<U>&& other)
		{
			delete _raw;
			U* temp = std::move(*other._raw).clone_new();
			_raw = dynamic_cast<T*>(temp);
			if (!_raw)
			{
				delete temp;
				throw Error(ErrorCode::BAD_CAST);
			}
			return *this;
		}

		T& operator*()
		{
			return *_raw;
		}

		const T& operator*() const
		{
			return *_raw;
		}

		T* operator->()
		{
			return _raw;
		}

		const T* operator->() const
		{
			return _raw;
		}

		template<typename U>
		U* as()
		{
			return dynamic_cast<U*>(_raw);
		}

		template<typename U>
		const U* as() const
		{
			return dynamic_cast<const U*>(_raw);
		}
	};

	template<typename T, typename... Args>
	inline Polymorphic<T> make_polymorphic(Args&&... args)
	{
		return Polymorphic(new T(std::forward<Args>(args)...));
	}

	template<typename U>
	inline Polymorphic<std::remove_cvref_t<U>> as_polymorphic(U&& obj)
	{
		return Polymorphic<std::remove_cvref_t<U>>(new std::remove_cvref_t<U>(std::forward<U>(obj)));
	}

	template<typename T, typename U> requires (PolymorphicBaseOf<std::remove_cvref_t<U>, T>)
	inline Polymorphic<T> as_polymorphic(U&& obj)
	{
		return Polymorphic<U>(new std::remove_cvref_t<U>(std::forward<U>(obj)));
	}
}

#define OLY_POLYMORPHIC_CLONE_DEFINITION(...)\
	public:\
		virtual OLY_FLATTEN(__VA_ARGS__)* clone_new() const&\
		{\
			return new OLY_FLATTEN(__VA_ARGS__)(*this);\
		}\
		virtual OLY_FLATTEN(__VA_ARGS__)* clone_new() &&\
		{\
			return new OLY_FLATTEN(__VA_ARGS__)(std::move(*this)); \
		}

#define OLY_POLYMORPHIC_CLONE_ABSTACT_DECLARATION(...)\
	public:\
		virtual OLY_FLATTEN(__VA_ARGS__)* clone_new() const& = 0; \
		virtual OLY_FLATTEN(__VA_ARGS__)* clone_new() && = 0;

#define OLY_POLYMORPHIC_CLONE_OVERRIDE(...)\
	public:\
		virtual OLY_FLATTEN(__VA_ARGS__)* clone_new() const& override\
		{\
			return new OLY_FLATTEN(__VA_ARGS__)(*this);\
		}\
		virtual OLY_FLATTEN(__VA_ARGS__)* clone_new() && override\
		{\
			return new OLY_FLATTEN(__VA_ARGS__)(std::move(*this));\
		}
