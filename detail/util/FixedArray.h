#pragma once

#include <stdexcept>
#include <string>

namespace oly
{
	template<typename T>
	class FixedArray
	{
		T* _ptr;
		const size_t _length;

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
			: _ptr(new T[length]), _length(length)
		{
		}

		FixedArray(const FixedArray& o)
			: _ptr(new T[o._length]), _length(o._length)
		{
			for (size_t i = 0; i < _length; ++i)
				_ptr[i] = o._ptr[i];
		}

		FixedArray(FixedArray&& o) noexcept
			: _ptr(o._ptr), _length(o._length)
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
	};
}
