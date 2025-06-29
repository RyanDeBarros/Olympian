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

		class Iterator
		{
			friend class FixedVector<T>;
			FixedVector<T>& vec;
			size_t i = 0;

			Iterator(FixedVector<T>& vec, size_t i) : vec(vec), i(i) {}

			size_t idx() const
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR);
				return i;
			}

		public:
			bool operator==(const Iterator& other) const { return &vec == &other.vec && i == other.i; }
			bool operator!=(const Iterator&) const = default;
			
			const T& operator*() const { return vec[idx()]; }
			T& operator*() { return vec[idx()]; }
			const T* operator->() const { return vec.data() + idx(); }
			T* operator->() { return vec.data() + idx(); }

			Iterator& operator++()
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR);
				++i;
				return *this;
			}
			Iterator operator++(int)
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR);
				Iterator copy = *this;
				++i;
				return copy;
			}
		};

		class ConstIterator
		{
			friend class FixedVector<T>;
			const FixedVector<T>& vec;
			size_t i = 0;

			ConstIterator(const FixedVector<T>& vec, size_t i) : vec(vec), i(i) {}

			size_t idx() const
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR);
				return i;
			}

		public:
			bool operator==(const ConstIterator& other) const { return &vec == &other.vec && i == other.i; }
			bool operator!=(const ConstIterator&) const = default;

			const T& operator*() const { return vec[idx()]; }
			const T* operator->() const { return vec.data() + idx(); }

			ConstIterator& operator++()
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR);
				++i;
				return *this;
			}
			ConstIterator operator++(int)
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR);
				ConstIterator copy = *this;
				++i;
				return copy;
			}
		};

		class ReverseIterator
		{
			friend class FixedVector<T>;
			FixedVector<T>& vec;
			size_t i = 0;

			ReverseIterator(FixedVector<T>& vec, size_t i) : vec(vec), i(i) {}

			size_t idx() const
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR);
				return vec.size() - 1 - i;
			}

		public:
			bool operator==(const ReverseIterator& other) const { return &vec == &other.vec && i == other.i; }
			bool operator!=(const ReverseIterator&) const = default;

			const T& operator*() const { return vec[idx()]; }
			T& operator*() { return vec[idx()]; }
			const T* operator->() const { return vec.data() + idx(); }
			T* operator->() { return vec.data() + idx(); }

			ReverseIterator& operator++()
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR);
				++i;
				return *this;
			}
			ReverseIterator operator++(int)
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR);
				ReverseIterator copy = *this;
				++i;
				return copy;
			}
		};

		class ConstReverseIterator
		{
			friend class FixedVector<T>;
			const FixedVector<T>& vec;
			size_t i = 0;

			ConstReverseIterator(const FixedVector<T>& vec, size_t i) : vec(vec), i(i) {}

			size_t idx() const
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR);
				return vec.size() - 1 - i;
			}

		public:
			bool operator==(const ConstReverseIterator& other) const { return &vec == &other.vec && i == other.i; }
			bool operator!=(const ConstReverseIterator&) const = default;

			const T& operator*() const { return vec[idx()]; }
			const T* operator->() const { return vec.data() + idx(); }

			ConstReverseIterator& operator++()
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR); 
				++i;
				return *this;
			}
			ConstReverseIterator operator++(int)
			{
				if (i >= vec._size)
					throw Error(ErrorCode::INVALID_ITERATOR);
				ConstReverseIterator copy = *this;
				++i;
				return copy;
			}
		};

		Iterator begin() { return Iterator(*this, 0); }
		Iterator end() { return Iterator(*this, _size); }
		ConstIterator begin() const { return ConstIterator(*this, 0); }
		ConstIterator end() const { return ConstIterator(*this, _size); }
		ConstIterator cbegin() const { return ConstIterator(*this, 0); }
		ConstIterator cend() const { return ConstIterator(*this, _size); }
		ReverseIterator rbegin() { return ReverseIterator(*this, 0); }
		ReverseIterator rend() { return ReverseIterator(*this, _size); }
		ConstReverseIterator rbegin() const { return ConstReverseIterator(*this, 0); }
		ConstReverseIterator rend() const { return ConstReverseIterator(*this, _size); }
		ConstReverseIterator crbegin() const { return ConstReverseIterator(*this, 0); }
		ConstReverseIterator crend() const { return ConstReverseIterator(*this, _size); }
	};
}
