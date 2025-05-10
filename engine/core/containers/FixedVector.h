#pragma once

#include "core/base/Errors.h"

namespace oly
{
	template<typename T>
	class FixedVector
	{
		size_t _size = 0;
		T* _arr = nullptr;

	public:
		FixedVector(size_t size)
			: _size(size), _arr(static_cast<T*>(operator new[](size * sizeof(T))))
		{
			for (size_t i = 0; i < size; ++i)
			{
				try
				{
					new (_arr + i) T();
				}
				catch (...)
				{
					for (size_t j = 0; j < i; ++j)
						_arr[j].~T();
					operator delete[](_arr);
					throw;
				}
			}
		}

		FixedVector(size_t size, const T& default_value)
			: _size(size), _arr(static_cast<T*>(operator new[](size * sizeof(T))))
		{
			for (size_t i = 0; i < size; ++i)
			{
				try
				{
					new (_arr + i) T(default_value);
				}
				catch (...)
				{
					for (size_t j = 0; j < i; ++j)
						_arr[j].~T();
					operator delete[](_arr);
					throw;
				}
			}
		}

		template<typename... Args>
		FixedVector(size_t size, const Args&... args)
			: _size(size), _arr(static_cast<T*>(operator new[](size * sizeof(T))))
		{
			for (size_t i = 0; i < size; ++i)
			{
				try
				{
					new (_arr + i) T(args...);
				}
				catch (...)
				{
					for (size_t j = 0; j < i; ++j)
						_arr[j].~T();
					operator delete[](_arr);
					throw;
				}
			}
		}

		FixedVector(const FixedVector<T>& other)
			: _size(other._size), _arr(static_cast<T*>(operator new[](other._size * sizeof(T))))
		{
			for (size_t i = 0; i < _size; ++i)
			{
				try
				{
					new (_arr + i) T(other._arr[i]);
				}
				catch (...)
				{
					for (size_t j = 0; j < i; ++j)
						_arr[j].~T();
					operator delete[](_arr);
					throw;
				}
			}
		}

		FixedVector(FixedVector<T>&& other) noexcept
			: _size(other._size), _arr(other._arr)
		{
			other._size = 0;
			other._arr = nullptr;
		}

		~FixedVector()
		{
			for (size_t i = 0; i < _size; ++i)
				_arr[i].~T();

			operator delete[](_arr);
		}

		FixedVector<T>& operator=(const FixedVector<T>& other)
		{
			if (this != &other)
			{
				for (size_t i = 0; i < _size; ++i)
					_arr[i].~T();

				operator delete[](_arr);

				_size = other._size;
				_arr = static_cast<T*>(operator new[](other._size * sizeof(T)));

				for (size_t i = 0; i < _size; ++i)
				{
					try
					{
						new (_arr + i) T(other._arr[i]);
					}
					catch (...)
					{
						for (size_t j = 0; j < i; ++j)
							_arr[j].~T();
						operator delete[](_arr);
						throw;
					}
				}
			}
			return *this;
		}

		FixedVector<T>& operator=(FixedVector<T>&& other) noexcept
		{
			if (this != &other)
			{
				for (size_t i = 0; i < _size; ++i)
					_arr[i].~T();

				operator delete[](_arr);

				_size = other._size;
				_arr = other._arr;
				other._size = 0;
				other._arr = nullptr;
			}
			return *this;
		}

		const T& operator[](size_t i) const
		{
			if (i >= _size)
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return _arr[i];
		}

		T& operator[](size_t i)
		{
			if (i >= _size)
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);
			return _arr[i];
		}

		size_t size() const
		{
			return _size;
		}
		
		const T* data() const
		{ 
			return _arr;
		}
		
		T* data()
		{
			return _arr;
		}
	};
}
