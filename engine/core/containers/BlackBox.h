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

	class NonCopyableBlackBox;

	class BlackBox
	{
		friend class NonCopyableBlackBox;

		void* ptr = nullptr;
		void(*deleter)(void*) = nullptr;
		void* (*copier)(const void*) = nullptr;

	public:
		BlackBox() = default;

		template<typename T>
		explicit BlackBox(T* raw)
		{
			ptr = static_cast<void*>(raw);
			deleter = internal::deleter<T>();
			copier = internal::copier<T>();
		}

		template<typename T>
		explicit BlackBox(T&& obj) noexcept
		{
			ptr = static_cast<void*>(new std::decay_t<T>(std::forward<T>(obj)));
			deleter = internal::deleter<std::decay_t<T>>();
			copier = internal::copier<std::decay_t<T>>();
		}

		~BlackBox()
		{
			if (ptr && deleter)
				deleter(ptr);
		}

		BlackBox(const BlackBox& other)
		{
			if (other.ptr && other.copier)
			{
				ptr = other.copier(other.ptr);
				deleter = other.deleter;
				copier = other.copier;
			}
		}

		BlackBox& operator=(const BlackBox& other)
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

		BlackBox(BlackBox&& other) noexcept
			: ptr(other.ptr), deleter(other.deleter), copier(other.copier)
		{
			other.ptr = nullptr;
			other.deleter = nullptr;
			other.copier = nullptr;
		}

		BlackBox& operator=(BlackBox&& other) noexcept
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

		explicit operator NonCopyableBlackBox() const;

		const void* raw() const { return ptr; }
		void* raw() { return ptr; }
		operator bool() const { return ptr; }
		template<typename T>
		const T* cast() const { return static_cast<const T*>(ptr); }
		template<typename T>
		T* cast() { return static_cast<T*>(ptr); }
	};

	template<typename T, typename... Args>
	inline BlackBox make_black_box(Args&&... args) noexcept
	{
		return BlackBox(new T(std::forward<Args>(args)...));
	}

	class NonCopyableBlackBox
	{
		void* ptr = nullptr;
		void(*deleter)(void*) = nullptr;

	public:
		NonCopyableBlackBox() = default;

		template<typename T>
		explicit NonCopyableBlackBox(T* raw)
		{
			ptr = static_cast<void*>(raw);
			deleter = internal::deleter<T>();
		}

		template<typename T>
		explicit NonCopyableBlackBox(T&& obj) noexcept
		{
			ptr = static_cast<void*>(new std::decay_t<T>(std::forward<T>(obj)));
			deleter = internal::deleter<std::decay_t<T>>();
		}

		~NonCopyableBlackBox()
		{
			if (ptr && deleter)
				deleter(ptr);
		}

		NonCopyableBlackBox(const NonCopyableBlackBox&) = delete;
		NonCopyableBlackBox& operator=(const NonCopyableBlackBox&) = delete;

		NonCopyableBlackBox(NonCopyableBlackBox&& other) noexcept
			: ptr(other.ptr), deleter(other.deleter)
		{
			other.ptr = nullptr;
			other.deleter = nullptr;
		}

		NonCopyableBlackBox& operator=(NonCopyableBlackBox&& other) noexcept
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

		explicit NonCopyableBlackBox(const BlackBox& bb)
		{
			if (bb.ptr && bb.copier)
			{
				ptr = bb.copier(bb.ptr);
				deleter = bb.deleter;
			}
		}

		explicit NonCopyableBlackBox(BlackBox&& bb)
			: ptr(bb.ptr), deleter(bb.deleter)
		{
			bb.ptr = nullptr;
			bb.deleter = nullptr;
			bb.copier = nullptr;
		}

		NonCopyableBlackBox& operator=(const BlackBox& bb)
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

		NonCopyableBlackBox& operator=(BlackBox&& bb) noexcept
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

	template<typename T, typename... Args>
	inline NonCopyableBlackBox make_non_copyable_black_box(Args&&... args) noexcept
	{
		return NonCopyableBlackBox(new T(std::forward<Args>(args)...));
	}

	inline BlackBox::operator NonCopyableBlackBox() const
	{
		return NonCopyableBlackBox(*this);
	}
}
