#pragma once

#include <stdexcept>
#include <span>

namespace oly
{
	struct DynamicArrayResizeStrategy
	{
		float mult = 1.5f;
		size_t increment = 1;
	};

	template<typename T, DynamicArrayResizeStrategy ResizeStrategy = DynamicArrayResizeStrategy()>
	class DynamicArray
	{
		static_assert(ResizeStrategy.mult >= 1.f && ResizeStrategy.increment > 0, "invalid resize strategy");

		size_t _size = 0;
		size_t _capacity = 0;
		T* _arr = nullptr;

	public:
		struct OutOfRangeError : public std::runtime_error
		{
			size_t size, index;

			OutOfRangeError(size_t size, size_t index)
				: index(index), size(size), std::runtime_error("index " + std::to_string(index) + " out of range for array size " + std::to_string(size))
			{
			}
		};

		DynamicArray() = default;

		DynamicArray(const DynamicArray& o)
			: _size(o._size), _capacity(o._capacity), _arr(new T[_capacity])
		{
			for (size_t i = 0; i < _size; ++i)
				_arr[i] = o._arr[i];
		}

		DynamicArray(DynamicArray&& o) noexcept
			: _size(o._size), _capacity(o._capacity), _arr(o._arr)
		{
			o._arr = nullptr;
			o._capacity = 0;
			o._size = 0;
		}

		~DynamicArray()
		{
			delete[] _arr;
		}

		DynamicArray& operator=(const DynamicArray& o)
		{
			if (this != &o)
			{
				delete[] _arr;
				_size = o._size;
				_capacity = o._capacity;
				_arr = new T[_capacity];
				for (size_t i = 0; i < _size; ++i)
					_arr[i] = o._arr[i];
			}

			return *this;
		}
		
		DynamicArray& operator=(DynamicArray&& o) noexcept
		{
			if (this != &o)
			{
				delete[] _arr;
				_size = o._size;
				_capacity = o._capacity;
				_arr = o._arr;
				o._arr = nullptr;
				o._capacity = 0;
				o._size = 0;
			}

			return *this;
		}

		const T& operator[](size_t i) const
		{
			if (i < _size)
				return _arr[i];
			else
				throw OutOfRangeError(_size, i);
		}
		
		T& operator[](size_t i) 
		{
			if (i < _size)
				return _arr[i];
			else
				throw OutOfRangeError(_size, i);
		}

		const T* data() const noexcept
		{
			return _arr;
		}

		T* data() noexcept
		{
			return _arr;
		}

		size_t size() const noexcept
		{
			return _size;
		}

		size_t capacity() const noexcept
		{
			return _capacity;
		}

		T* begin() noexcept
		{
			return _arr;
		}

		T* end() noexcept
		{
			return _arr + _size;
		}

		const T* begin() const noexcept
		{
			return _arr;
		}

		const T* end() const noexcept
		{
			return _arr + _size;
		}

		const T* cbegin() const noexcept
		{
			return _arr;
		}

		const T* cend() const noexcept
		{
			return _arr + _size;
		}

		operator std::span<T>() noexcept
		{
			return std::span<T>(_arr, _size);
		}

		operator std::span<const T>() const noexcept
		{
			return std::span<const T>(_arr, _size);
		}

		void push_back(T obj)
		{
			if (_size == _capacity)
				reallocate();

			_arr[_size++] = std::move(obj);
		}

		void insert(size_t pos, T obj)
		{
			if (pos > _size)
				throw OutOfRangeError(_size, pos);

			if (_size == _capacity)
				reallocate();
			
			for (long i = _size - 1; i >= pos; --i)
				_arr[i + 1] = std::move(_arr[i]);
			_arr[pos] = std::move(obj);
			++_size;
		}
		
		void erase(size_t pos)
		{
			if (pos >= _size)
				throw OutOfRangeError(_size, pos);

			for (long i = pos; i < _size - 1; ++i)
				_arr[i] = std::move(_arr[i + 1]);
			--_size;
		}

		void clear()
		{
			delete[] _arr;
			_arr = nullptr;
			_capacity = 0;
			_size = 0;
		}

		T* release(size_t& out_size)
		{
			if (_size == 0)
			{
				out_size = 0;
				return nullptr;
			}

			if (_size != _capacity)
			{
				T* new_arr = new T[_size];
				for (size_t i = 0; i < _size; ++i)
					new_arr[i] = std::move(_arr[i]);

				delete[] _arr;
				_arr = new_arr;
				_capacity = _size;
			}

			out_size = _size;
			T* ptr = _arr;
			_arr = nullptr;
			_capacity = 0;
			_size = 0;
			return ptr;
		}

		T* release(size_t& out_size, size_t& out_capacity)
		{
			out_size = _size;
			out_capacity = _capacity;
			T* ptr = _arr;
			_arr = nullptr;
			_capacity = 0;
			_size = 0;
			return ptr;
		}

	private:
		void reallocate()
		{
			reserve(static_cast<size_t>(_capacity * ResizeStrategy.mult) + ResizeStrategy.increment);
		}

	public:
		void reserve(size_t new_capacity)
		{
			if (new_capacity > _capacity)
			{
				T* new_arr = new T[new_capacity];
				for (size_t i = 0; i < _size; ++i)
					new_arr[i] = std::move(_arr[i]);

				delete[] _arr;
				_arr = new_arr;
				_capacity = new_capacity;
			}
		}
	};
}
