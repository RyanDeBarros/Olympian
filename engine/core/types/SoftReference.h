#pragma once

#include <memory>

namespace oly
{
	// LATER add thread-safe soft references that use atomic bool statuses.

	/*
	 * Soft references allow for safe, non-dangling, non-owning references to objects of any allocation type (stack, unique, shared, etc.) using CRTP instead of inheritance.
	 * Optimal way of dereferencing a soft reference is via:
	 * if (auto r = ref.get())
	 *     r->do_something();
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

		friend class SoftReferenceBase<T>;
		friend class ConstSoftReference<T>;
		SoftReference(T* ref, const std::shared_ptr<bool>& status) : ref(ref), status(status) {}

	public:
		SoftReference() = default;

		SoftReference(std::nullptr_t) {}
		SoftReference& operator=(std::nullptr_t) { ref = nullptr; status.reset(); return *this; }

		const T* get() const { return status && *status ? ref : nullptr; }
		T* get() { return status && *status ? ref : nullptr; }

		operator bool() const { return status && *status; }
		const T& operator*() const { return *get(); }
		T& operator*() { return *get(); }
		const T* operator->() const { return get(); }
		T* operator->() { return get(); }
	};

	template<typename T>
	class ConstSoftReference
	{
		const T* ref = nullptr;
		std::shared_ptr<bool> status = nullptr;

		friend class SoftReferenceBase<T>;
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

		const T* get() const { return status && *status ? ref : nullptr; }

		operator bool() const { return status && *status; }
		const T& operator*() const { return *get(); }
		const T* operator->() const { return get(); }
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

	private:
		friend T;
		SoftReference<T> ref(T* obj) const { return SoftReference<T>(obj, status); }
		ConstSoftReference<T> cref(const T* obj) const { return ConstSoftReference<T>(obj, status); }
		ConstSoftReference<T> cref(T* obj) const { return ConstSoftReference<T>(obj, status); }
	};
}

template<typename T>
struct std::hash<oly::SoftReference<T>>
{
	size_t operator()(const oly::SoftReference<T>& ref) const
	{
		return hash<T*>{}(ref.get());
	}
};

template<typename T>
struct std::hash<oly::ConstSoftReference<T>>
{
	size_t operator()(const oly::ConstSoftReference<T>& ref) const
	{
		return hash<const T*>{}(ref.get());
	}
};
