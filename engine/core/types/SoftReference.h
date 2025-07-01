#pragma once

#include "core/types/Meta.h"

#include <memory>

namespace oly
{
	/*
	 * Soft references allow for safe, non-dangling, non-owning references to objects of any allocation type (stack, unique, shared, etc.) using CRTP instead of inheritance.
	 * Optimal way of dereferencing a soft reference is via:
	 * if (auto r = ref.get())
	 *     r->do_something();
	 * 
	 * Note that soft references are NOT thread-safe. In fact, their design does not allow for multi-threading in any capacity, since the internal pointer may be dangling after checking if status is alive.
	 */

	template<typename T>
	class SoftReferenceBase;
	template<typename T>
	class ConstSoftReference;

	template<typename T>
	class SoftReference
	{
		T* ref = nullptr;
		std::shared_ptr<bool> status = nullptr;

		template<typename>
		friend class SoftReference;
		template<typename>
		friend class SoftReferenceBase;
		template<typename>
		friend class ConstSoftReference;
		SoftReference(T* ref, const std::shared_ptr<bool>& status) : ref(ref), status(status) {}

	public:
		SoftReference() = default;

		SoftReference(std::nullptr_t) {}
		SoftReference& operator=(std::nullptr_t) { ref = nullptr; status.reset(); return *this; }

		template<PointerConvertibleTo<T> U>
		SoftReference(const SoftReference<U>& other) : ref(static_cast<T*>(other.ref)), status(other.status) {}
		template<PointerConvertibleTo<T> U>
		SoftReference(SoftReference<U>&& other) : ref(static_cast<T*>(other.ref)), status(std::move(other.status)) {}

		T* get() const { return status && *status ? ref : nullptr; }

		operator bool() const { return status && *status; }
		T& operator*() const { return *get(); }
		T* operator->() const { return get(); }

		bool operator==(const SoftReference<T>& other) const { return ref == other.ref; }
		bool operator!=(const SoftReference<T>& other) const { return ref != other.ref; }

		template<typename U>
		SoftReference<U> cast_dynamic() const
		{
			if (status && *status)
				if (U* cast = dynamic_cast<U*>(ref))
					return SoftReference<U>(cast, status);
			return nullptr;
		}

		template<typename U>
		SoftReference<U> cast_static() const
		{
			return SoftReference<U>(static_cast<U*>(ref), status);
		}

		size_t hash() const { return std::hash<T*>{}(ref); }
	};

	template<typename T>
	class ConstSoftReference
	{
		const T* ref = nullptr;
		std::shared_ptr<bool> status = nullptr;

		template<typename>
		friend class ConstSoftReference;
		template<typename>
		friend class SoftReferenceBase;
		ConstSoftReference(const T* ref, const std::shared_ptr<bool>& status) : ref(ref), status(status) {}
		ConstSoftReference(T* ref, const std::shared_ptr<bool>& status) : ref(ref), status(status) {}

	public:
		ConstSoftReference() = default;

		ConstSoftReference(std::nullptr_t) {}
		ConstSoftReference& operator=(std::nullptr_t) { ref = nullptr; status.reset(); return *this; }

		ConstSoftReference(const SoftReference<T>& other) : ref(other.ref), status(other.status) {}
		ConstSoftReference(SoftReference<T>&& other) noexcept : ref(other.ref), status(std::move(other.status)) {}
		ConstSoftReference& operator=(const SoftReference<T>& other) { ref = other.ref; status = other.status; return *this; }
		ConstSoftReference& operator=(SoftReference<T>&& other) noexcept { ref = other.ref; status = std::move(other.status); return *this; }

		template<PointerConvertibleTo<T> U>
		ConstSoftReference(const ConstSoftReference<U>& other) : ref(static_cast<const T*>(other.ref)), status(other.status) {}
		template<PointerConvertibleTo<T> U>
		ConstSoftReference(ConstSoftReference<U>&& other) : ref(static_cast<const T*>(other.ref)), status(std::move(other.status)) {}

		template<PointerConvertibleTo<T> U>
		ConstSoftReference(const SoftReference<U>& other) : ref(static_cast<const T*>(other.ref)), status(other.status) {}
		template<PointerConvertibleTo<T> U>
		ConstSoftReference(SoftReference<U>&& other) : ref(static_cast<const T*>(other.ref)), status(std::move(other.status)) {}

		const T* get() const { return status && *status ? ref : nullptr; }

		operator bool() const { return status && *status; }
		const T& operator*() const { return *get(); }
		const T* operator->() const { return get(); }

		bool operator==(const ConstSoftReference<T>& other) const { return ref == other.ref; }
		bool operator!=(const ConstSoftReference<T>& other) const { return ref != other.ref; }
		bool operator==(const SoftReference<T>& other) const { return ref == other.ref; }
		bool operator!=(const SoftReference<T>& other) const { return ref != other.ref; }

		template<typename U>
		ConstSoftReference<U> cast_dynamic() const
		{
			if (status && *status)
				if (U* cast = dynamic_cast<const U*>(ref))
					return ConstSoftReference<U>(cast, status);
			return nullptr;
		}

		template<typename U>
		ConstSoftReference<U> cast_static() const
		{
			return ConstSoftReference<U>(static_cast<const U*>(ref), status);
		}

		size_t hash() const { return std::hash<const T*>{}(ref); }
	};

	/*
	 * To use soft references on a class, make SoftReferenceBase a private data member (_ref) of that class, and expose references via public methods:
	 * ConstSoftReference<Class> cref() const { return _ref.cref(this); }
	 * ConstSoftReference<Class> ref() const { return _ref.cref(this); }
	 * SoftReference<Class> ref() { return _ref.ref(this); }
	 */
	template<typename T>
	class SoftReferenceBase
	{
		std::shared_ptr<bool> status = std::make_shared<bool>(true);

	public:
		SoftReferenceBase() = default;
		SoftReferenceBase(const SoftReferenceBase<T>&) : status(std::make_shared<bool>(true)) {}
		SoftReferenceBase(SoftReferenceBase<T>&&) noexcept : status(std::make_shared<bool>(true)) {}
		~SoftReferenceBase() { *status = false; }
		SoftReferenceBase& operator=(const SoftReferenceBase<T>&) { return *this; }
		SoftReferenceBase& operator=(SoftReferenceBase<T>&&) { return *this; }

		SoftReference<T> ref(T* obj) const { return SoftReference<T>(obj, status); }
		ConstSoftReference<T> cref(const T* obj) const { return ConstSoftReference<T>(obj, status); }
		ConstSoftReference<T> cref(T* obj) const { return ConstSoftReference<T>(obj, status); }

		template<PointerConvertibleTo<T> U>
		SoftReference<U> ref(U* obj) const { return SoftReference<U>(obj, status); }
		template<PointerConvertibleTo<T> U>
		ConstSoftReference<U> cref(const U* obj) const { return ConstSoftReference<U>(obj, status); }
		template<PointerConvertibleTo<T> U>
		ConstSoftReference<U> cref(U* obj) const { return ConstSoftReference<U>(obj, status); }

		size_t hash() const { return std::hash<std::shared_ptr<bool>>{}(status); }
	};

#define OLY_SOFT_REFERENCE_BASE_DECLARATION(Class)\
	protected:\
		oly::SoftReferenceBase<Class> _ref_base;\
	\
	public:\
		oly::ConstSoftReference<Class> ref() const { return _ref_base.cref(this); }\
		oly::ConstSoftReference<Class> cref() const { return _ref_base.cref(this); }\
		oly::SoftReference<Class> ref() { return _ref_base.ref(this); }

#define OLY_SOFT_REFERENCE_PUBLIC(Class)\
	public:\
		oly::ConstSoftReference<Class> ref() const { return _ref_base.cref(this); }\
		oly::ConstSoftReference<Class> cref() const { return _ref_base.cref(this); }\
		oly::SoftReference<Class> ref() { return _ref_base.ref(this); }

}

template<typename T>
struct std::hash<oly::SoftReference<T>>
{
	size_t operator()(const oly::SoftReference<T>& ref) const
	{
		return ref.hash();
	}
};

template<typename T>
struct std::hash<oly::ConstSoftReference<T>>
{
	size_t operator()(const oly::ConstSoftReference<T>& ref) const
	{
		return ref.hash();
	}
};

template<typename T>
struct std::hash<oly::SoftReferenceBase<T>>
{
	size_t operator()(const oly::SoftReferenceBase<T>& base) const
	{
		return base.hash();
	}
};
