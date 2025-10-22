#pragma once

#include "core/base/Errors.h"
#include "core/types/Meta.h"

namespace oly
{
	template<typename T>
	class CopyPtr
	{
		T* _ptr = nullptr;

		template<typename>
		friend class CopyPtr;

	public:
		CopyPtr() = default;

		explicit CopyPtr(T* ptr) : _ptr(ptr) {}
		
		explicit CopyPtr(const T& obj) : _ptr(new T(obj)) {}
		
		explicit CopyPtr(T&& obj) noexcept : _ptr(new T(std::move(obj))) {}

		CopyPtr(const CopyPtr<T>& other) : _ptr(other._ptr ? new T(*other._ptr) : nullptr) {}

		CopyPtr(CopyPtr<T>&& other) noexcept : _ptr(other._ptr) { other._ptr = nullptr; }

		template<PolymorphicBaseOf<T> U>
		CopyPtr(const CopyPtr<U>& other)
		{
			U* raw = new U(*other._ptr);
			_ptr = dynamic_cast<T*>(raw);
			if (!_ptr)
			{
				delete raw;
				throw Error(ErrorCode::BAD_CAST);
			}
		}

		template<PolymorphicBaseOf<T> U>
		CopyPtr(CopyPtr<U>&& other)
		{
			_ptr = dynamic_cast<T*>(other._ptr);
			if (_ptr)
				other._ptr = nullptr;
			else
			{
				delete other._ptr;
				other._ptr = nullptr;
				throw Error(ErrorCode::BAD_CAST);
			}
		}

		~CopyPtr() { if (_ptr) delete _ptr; }

		CopyPtr<T>& operator=(const CopyPtr<T>& other)
		{
			if (this != &other)
			{
				if (other._ptr)
				{
					if (_ptr)
						*_ptr = *other._ptr;
					else
						_ptr = new T(*other._ptr);
				}
				else if (_ptr)
				{
					delete _ptr;
					_ptr = nullptr;
				}
			}
			return *this;
		}

		CopyPtr<T>& operator=(CopyPtr<T>&& other) noexcept
		{
			if (this != &other)
			{
				if (_ptr)
					delete _ptr;
				_ptr = other._ptr;
				other._ptr = nullptr;
			}
			return *this;
		}

		template<PolymorphicBaseOf<T> U>
		CopyPtr<T>& operator=(const CopyPtr<U>& other)
		{
			if (reinterpret_cast<const void*>(this) != reinterpret_cast<const void*>(&other))
			{
				if (_ptr)
				{
					delete _ptr;
					_ptr = nullptr;
				}

				if (other._ptr)
				{
					U* raw = new U(*other._ptr);
					_ptr = dynamic_cast<T*>(raw);
					if (!_ptr)
					{
						delete raw;
						throw Error(ErrorCode::BAD_CAST);
					}
				}
			}
			return *this;
		}

		template<PolymorphicBaseOf<T> U>
		CopyPtr<T>& operator=(CopyPtr<U>&& other) noexcept
		{
			if (reinterpret_cast<const void*>(this) != reinterpret_cast<const void*>(&other))
			{
				if (_ptr)
				{
					delete _ptr;
					_ptr = nullptr;
				}

				_ptr = dynamic_cast<T*>(other._ptr);
				if (_ptr)
					other._ptr = nullptr;
				else
				{
					delete other._ptr;
					other._ptr = nullptr;
					throw Error(ErrorCode::BAD_CAST);
				}
			}
			return *this;
		}

		void reset(T* ptr = nullptr) { if (_ptr && _ptr != ptr) delete _ptr; _ptr = ptr; }

		const T* get() const { return _ptr; }

		T* get() { return _ptr; }

		const T& operator*() const { if (_ptr) return *_ptr; else throw Error(ErrorCode::NULL_POINTER); }

		T& operator*() { if (_ptr) return *_ptr; else throw Error(ErrorCode::NULL_POINTER); }

		const T* operator->() const { if (_ptr) return _ptr; else throw Error(ErrorCode::NULL_POINTER); }

		T* operator->() { if (_ptr) return _ptr; else throw Error(ErrorCode::NULL_POINTER); }

		operator bool() const { return _ptr; }
	};

	template<typename T, typename... Args>
	inline CopyPtr<T> make_copy_ptr(Args&&... args) { return CopyPtr<T>(new T(std::forward<Args>(args)...)); }
}
