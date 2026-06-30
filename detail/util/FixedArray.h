#pragma once

#include <span>
#include <stdexcept>
#include <string>

#include "util/DynamicArray.h"

namespace oly
{
	template<typename T>
	class FixedArray
	{
		const size_t _length = 0;
		T* _ptr = nullptr;

	public:
		struct LengthMismatchError : public std::runtime_error
		{
			size_t expected_length, given_length;

			LengthMismatchError(size_t expected_length, size_t given_length)
				: expected_length(expected_length), given_length(given_length), std::runtime_error("provided length " + std::to_string(given_length) + " does not match array length " + std::to_string(expected_length))
			{
			}
		};

		struct OutOfRangeError : public std::runtime_error
		{
			size_t length, index;

			OutOfRangeError(size_t length, size_t index)
				: index(index), length(length), std::runtime_error("index " + std::to_string(index) + " out of range for array length " + std::to_string(length))
			{
			}
		};

		explicit FixedArray(size_t length)
			: _length(length), _ptr(new T[_length])
		{
		}

		FixedArray(size_t length, const T& value)
			: _length(length), _ptr(new T[_length])
		{
			for (size_t i = 0; i < length; ++i)
				_ptr[i] = value;
		}

		FixedArray(T* raw_array, size_t length)
			: _length(length), _ptr(raw_array)
		{
		}

		template<DynamicArrayResizeStrategy ResizeStrategy>
		FixedArray(DynamicArray<T, ResizeStrategy>&& array)
			: _length(array.size())
		{
			size_t sz = _length;
			_ptr = array.release(sz);

			if (_length == 0 || sz != _length)
			{
				delete[] _ptr;
				_ptr = nullptr;
			}

			if (sz != _length)
				throw LengthMismatchError(_length, sz);
		}

		FixedArray(std::initializer_list<T> init)
			: _length(init.size()), _ptr(new T[_length])
		{
			size_t i = 0;
			for (const T& obj : init)
				_ptr[i++] = obj;
		}
		
		FixedArray(const FixedArray& o)
			: _length(o._length), _ptr(new T[o._length])
		{
			for (size_t i = 0; i < _length; ++i)
				_ptr[i] = o._ptr[i];
		}

		FixedArray(FixedArray&& o) noexcept
			: _length(o._length), _ptr(o._ptr)
		{
			o._ptr = nullptr;
		}

		~FixedArray()
		{
			delete[] _ptr;
		}

		FixedArray& operator=(const FixedArray& o)
		{
			if (this != &o)
			{
				if (_length == o._length)
				{
					for (size_t i = 0; i < _length; ++i)
						_ptr[i] = o._ptr[i];
				}
				else
					throw LengthMismatchError(_length, o._length);
			}

			return *this;
		}

		FixedArray& operator=(FixedArray&& o)
		{
			if (this != &o)
			{
				if (_length == o._length)
				{
					delete[] _ptr;
					_ptr = o._ptr;
					o._ptr = nullptr;
				}
				else
					throw LengthMismatchError(_length, o._length);
			}

			return *this;
		}

		const T* data() const noexcept
		{
			return _ptr;
		}

		T* data() noexcept
		{
			return _ptr;
		}

		size_t length() const noexcept
		{
			return _length;
		}

		const T& operator[](size_t i) const
		{
			if (i < _length)
				return _ptr[i];
			else
				throw OutOfRangeError(_length, i);
		}

		T& operator[](size_t i)
		{
			if (i < _length)
				return _ptr[i];
			else
				throw OutOfRangeError(_length, i);
		}

		T* begin() noexcept
		{
			return _ptr;
		}

		T* end() noexcept
		{
			return _ptr + _length;
		}

		const T* begin() const noexcept
		{
			return _ptr;
		}

		const T* end() const noexcept
		{
			return _ptr + _length;
		}

		const T* cbegin() const noexcept
		{
			return _ptr;
		}

		const T* cend() const noexcept
		{
			return _ptr + _length;
		}

		operator std::span<T>() noexcept
		{
			return std::span<T>(_ptr, _length);
		}

		operator std::span<const T>() const noexcept
		{
			return std::span<const T>(_ptr, _length);
		}
	};
}
