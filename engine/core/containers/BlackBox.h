#pragma once

#include <utility>

namespace oly
{
	/*
	 * BlackBox wraps a type-erased, heap-allocated object.
	 */

	namespace internal
	{
		template<typename T>
		void(*deleter())(void*)
		{
			return [](void* ptr) { delete static_cast<T*>(ptr); };
		}

		template<typename T>
		void*(*copier())(const void*)
		{
			return [](const void* ptr) { return static_cast<void*>(new T(*static_cast<const T*>(ptr))); };
		}
	}

	template<bool CanCopy = false>
	class BlackBox;

	template<>
	class BlackBox<true>
	{
		friend class BlackBox<false>;

		void* ptr = nullptr;
		void(*deleter)(void*) = nullptr;
		void* (*copier)(const void*) = nullptr;

	public:
		BlackBox() = default;

		template<typename T, typename... Args>
		explicit BlackBox(Args&&... args) noexcept
		{
			ptr = static_cast<void*>(new T(std::forward<Args>(args)...));
			deleter = internal::deleter<T>();
			copier = internal::copier<T>();
		}

		template<typename T>
		explicit BlackBox(T* raw)
		{
			ptr = static_cast<void*>(raw);
			deleter = internal::deleter<T>();
			copier = internal::copier<T>();
		}

		template<typename T>
		explicit BlackBox(const T& obj)
		{
			ptr = static_cast<void*>(new T(obj));
			deleter = internal::deleter<T>();
			copier = internal::copier<T>();
		}

		template<typename T>
		explicit BlackBox(T&& obj) noexcept
		{
			ptr = static_cast<void*>(new T(std::move(obj)));
			deleter = internal::deleter<T>();
			copier = internal::copier<T>();
		}

		~BlackBox()
		{
			if (ptr && deleter)
				deleter(ptr);
		}

		BlackBox(const BlackBox<true>& other)
		{
			if (other.ptr && other.copier)
			{
				ptr = other.copier(other.ptr);
				deleter = other.deleter;
				copier = other.copier;
			}
		}

		BlackBox<true>& operator=(const BlackBox<true>& other)
		{
			if (this != &other)
			{
				if (ptr && deleter)
					deleter(ptr);
				if (other.ptr && other.copier)
				{
					ptr = other.copier(other.ptr);
					deleter = other.deleter;
					copier = other.copier;
				}
				else
				{
					ptr = nullptr;
					deleter = nullptr;
					copier = nullptr;
				}
			}
			return *this;
		}

		BlackBox(BlackBox<true>&& other) noexcept
			: ptr(other.ptr), deleter(other.deleter), copier(other.copier)
		{
			other.ptr = nullptr;
			other.deleter = nullptr;
			other.copier = nullptr;
		}

		BlackBox<true>& operator=(BlackBox<true>&& other) noexcept
		{
			if (this != &other)
			{
				if (ptr && deleter)
					deleter(ptr);
				ptr = other.ptr;
				deleter = other.deleter;
				copier = other.copier;
				other.ptr = nullptr;
				other.deleter = nullptr;
				other.copier = nullptr;
			}
			return *this;
		}

		explicit operator BlackBox<false>() const;

		const void* raw() const { return ptr; }
		void* raw() { return ptr; }
		operator bool() const { return ptr; }
		template<typename T>
		const T* cast() const { return static_cast<const T*>(ptr); }
		template<typename T>
		T* cast() { return static_cast<T*>(ptr); }
	};

	template<>
	class BlackBox<false>
	{
		void* ptr = nullptr;
		void(*deleter)(void*) = nullptr;

	public:
		BlackBox() = default;

		template<typename T, typename... Args>
		explicit BlackBox(Args&&... args) noexcept
		{
			ptr = static_cast<void*>(new T(std::forward<Args>(args)...));
			deleter = internal::deleter<T>();
		}

		template<typename T>
		explicit BlackBox(T* raw)
		{
			ptr = static_cast<void*>(raw);
			deleter = internal::deleter<T>();
		}

		template<typename T>
		explicit BlackBox(const T& obj)
		{
			ptr = static_cast<void*>(new T(obj));
			deleter = internal::deleter<T>();
		}

		template<typename T>
		explicit BlackBox(T&& obj) noexcept
		{
			ptr = static_cast<void*>(new T(std::move(obj)));
			deleter = internal::deleter<T>();
		}

		~BlackBox()
		{
			if (ptr && deleter)
				deleter(ptr);
		}

		BlackBox(const BlackBox<false>&) = delete;
		BlackBox<false>& operator=(const BlackBox<false>&) = delete;

		BlackBox(BlackBox<false>&& other) noexcept
			: ptr(other.ptr), deleter(other.deleter)
		{
			other.ptr = nullptr;
			other.deleter = nullptr;
		}

		BlackBox<false>& operator=(BlackBox<false>&& other) noexcept
		{
			if (this != &other)
			{
				if (ptr && deleter)
					deleter(ptr);
				ptr = other.ptr;
				deleter = other.deleter;
				other.ptr = nullptr;
				other.deleter = nullptr;
			}
			return *this;
		}

		explicit BlackBox(const BlackBox<true>& bb)
		{
			if (bb.ptr && bb.copier)
			{
				ptr = bb.copier(bb.ptr);
				deleter = bb.deleter;
			}
		}

		explicit BlackBox(BlackBox<true>&& bb)
			: ptr(bb.ptr), deleter(bb.deleter)
		{
			bb.ptr = nullptr;
			bb.deleter = nullptr;
			bb.copier = nullptr;
		}

		BlackBox<false>& operator=(const BlackBox<true>& bb)
		{
			if (ptr && deleter)
				deleter(ptr);
			if (bb.ptr && bb.copier)
			{
				ptr = bb.copier(bb.ptr);
				deleter = bb.deleter;
			}
			else
			{
				ptr = nullptr;
				deleter = nullptr;
			}
			return *this;
		}

		BlackBox<false>& operator=(BlackBox<true>&& bb) noexcept
		{
			if (ptr && deleter)
				deleter(ptr);
			ptr = bb.ptr;
			deleter = bb.deleter;
			bb.ptr = nullptr;
			bb.deleter = nullptr;
			bb.copier = nullptr;
			return *this;
		}

		const void* raw() const { return ptr; }
		void* raw() { return ptr; }
		operator bool() const { return ptr; }
		template<typename T>
		const T* cast() const { return static_cast<const T*>(ptr); }
		template<typename T>
		T* cast() { return static_cast<T*>(ptr); }
	};

	inline BlackBox<true>::operator BlackBox<false>() const
	{
		return BlackBox<false>(*this);
	}
}
