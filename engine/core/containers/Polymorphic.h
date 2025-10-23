#pragma once

#include "core/base/Errors.h"
#include "core/types/Meta.h"

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

		Polymorphic(const T& obj)
			: _raw(new T(obj))
		{
		}

		Polymorphic(T&& obj)
			: _raw(new T(std::move(obj)))
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

		Polymorphic& operator=(const T& obj)
		{
			*_raw = obj;
			return *this;
		}

		Polymorphic& operator=(T&& obj)
		{
			*_raw = std::move(obj);
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
	};
}

#define OLY_POLYMORPHIC_CLONE_DEFINITION(Base)\
	public:\
	virtual Base* clone_new() const&\
	{\
		return new Base(*this);\
	}\
	virtual Base* clone_new() &&\
	{\
		return new Base(std::move(*this)); \
	}

#define OLY_POLYMORPHIC_CLONE_OVERRIDE(Derived)\
	public:\
	virtual Derived* clone_new() const& override\
	{\
		return new Derived(*this);\
	}\
	virtual Derived* clone_new() && override\
	{\
		return new Derived(std::move(*this));\
	}
