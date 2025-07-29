#pragma once

#include "core/base/Errors.h"
#include "core/algorithms/STLUtils.h"

#include <stack>
#include <unordered_set>
#include <memory>

namespace oly
{
	template<typename Object>
	struct SmartPoolBase
	{
		using type = Object;
	};

	template<typename Object>
	using SmartPoolBaseType = typename SmartPoolBase<Object>::type;

	template<typename T1, typename T2>
	constexpr bool shares_smart_pool_base = std::is_same_v<SmartPoolBaseType<std::decay_t<T1>>, SmartPoolBaseType<std::decay_t<T2>>>;

#define OLY_SMART_POOL_BASE(Class, Base) template<> struct oly::SmartPoolBase<Class> { using type = Base;\
		static_assert(std::is_base_of_v<Base, Class>, "OLY_SMART_POOL_BASE: Class must derive from base."); };

	template<typename>
	struct SmartReference;

	namespace internal
	{
		struct IPool
		{
			virtual ~IPool() = default;
			virtual void clean() = 0;
			virtual void clear() = 0;
		};

		class PoolBatch
		{
			std::unordered_set<IPool*> pools;

			PoolBatch() = default;
			PoolBatch(const PoolBatch&) = delete;
			PoolBatch(PoolBatch&&) = delete;
			~PoolBatch() = default;

		public:
			static PoolBatch& instance()
			{
				static PoolBatch batch;
				return batch;
			}

			void insert(IPool* pool) { pools.insert(pool); }
			void remove(IPool* pool) { pools.erase(pool); }

			void clean()
			{
				for (IPool* pool : pools)
					pool->clean();
			}

			void clear()
			{
				for (IPool* pool : pools)
					pool->clear();
				pools.clear();
			}
		};

		struct SmartReferenceLink
		{
			SmartReferenceLink* prev = nullptr;
			SmartReferenceLink* next = nullptr;
		};

		// LATER multi-threading and thead safety
		template<typename Object>
		class SmartReferencePool : public IPool
		{
			std::vector<std::unique_ptr<Object>> objects;
			std::stack<size_t> unoccupied;
			std::unordered_set<size_t> marked_for_deletion;
			std::vector<SmartReferenceLink*> reference_heads;

			SmartReferencePool() { PoolBatch::instance().insert(this); }
			SmartReferencePool(const SmartReferencePool<Object>&) = delete;
			SmartReferencePool(SmartReferencePool<Object>&&) = delete;
			~SmartReferencePool() { PoolBatch::instance().remove(this); clean(); }

		public:
			static SmartReferencePool& instance()
			{
				static SmartReferencePool pool;
				return pool;
			}

			virtual void clean() override
			{
				for (size_t idx : marked_for_deletion)
				{
					while (reference_heads[idx])
					{
						reference_heads[idx]->prev = nullptr;
						SmartReferenceLink* next = reference_heads[idx]->next;
						reference_heads[idx]->next = nullptr;
						reference_heads[idx] = next;
					}
				}

				marked_for_deletion.clear();
			}

		private:
			friend class PoolBatch;
			void clear() override;

			template<typename>
			friend struct SmartReference;

			size_t init_slot()
			{
				return init_slot(std::make_unique<Object>());
			}

			template<typename T, typename = std::enable_if_t<shares_smart_pool_base<Object, T>>>
			size_t init_slot(const T& obj)
			{
				return init_slot(std::make_unique<T>(obj));
			}

			template<typename T, typename = std::enable_if_t<shares_smart_pool_base<Object, T>>>
			size_t init_slot(T&& obj)
			{
				return init_slot(std::make_unique<T>(std::move(obj)));
			}

			template<typename T, typename = std::enable_if_t<shares_smart_pool_base<Object, T>>>
			size_t init_slot(std::unique_ptr<T>&& obj_ptr)
			{
				if (unoccupied.empty())
				{
					objects.push_back(std::move(obj_ptr));
					reference_heads.push_back(nullptr);
					return objects.size() - 1;
				}
				else
				{
					size_t next_slot = unoccupied.top();
					objects[next_slot] = std::move(obj_ptr);
					unoccupied.pop();
					return next_slot;
				}
			}

			void mark_for_deletion(size_t idx)
			{
				marked_for_deletion.insert(idx);
			}
			
			void unmark_for_deletion(size_t idx)
			{
				marked_for_deletion.erase(idx);
			}
			
			bool is_marked_for_deletion(size_t idx) const
			{
				return marked_for_deletion.count(idx);
			}
			
			void increment_references(size_t idx, SmartReferenceLink* link)
			{
				if (reference_heads[idx])
				{
					auto* next = reference_heads[idx]->next;
					reference_heads[idx]->next = link;
					link->prev = reference_heads[idx];
					if (next)
					{
						link->next = next;
						next->prev = link;
					}
					else
					{
						link->next = reference_heads[idx];
						reference_heads[idx]->prev = link;
					}
				}
				else
					reference_heads[idx] = link;
			}

			void decrement_references(size_t idx, SmartReferenceLink* link)
			{
				SmartReferenceLink* prev = link->prev;
				SmartReferenceLink* next = link->next;

				if (next == prev)
				{
					if (next)
					{
						next->prev = nullptr;
						next->next = nullptr;
					}
				}
				else
				{
					if (next)
						next->prev = prev;
					if (prev)
						prev->next = next;
				}

				if (reference_heads[idx] == link)
					reference_heads[idx] = next;

				link->prev = nullptr;
				link->next = nullptr;

				if (!reference_heads[idx])
				{
					marked_for_deletion.erase(idx);
					unoccupied.push(idx);
				}
			}

			void replace_references(size_t idx, SmartReferenceLink* existing, SmartReferenceLink* with)
			{
				SmartReferenceLink* prev = existing->prev;
				SmartReferenceLink* next = existing->next;

				existing->prev = nullptr;
				existing->next = nullptr;

				with->prev = prev;
				with->next = next;

				if (prev)
					prev->next = with;
				if (next)
					next->prev = with;

				if (reference_heads[idx] == existing)
					reference_heads[idx] = with;
			}
		};

		struct RefInit {};
		struct RefDefault {};
	}

	constexpr internal::RefInit REF_INIT;
	constexpr internal::RefDefault REF_DEFAULT;

	template<typename Object>
	struct SmartReference : private internal::SmartReferenceLink
	{
		using PoolBase = SmartPoolBaseType<Object>;

	private:
		friend class internal::SmartReferencePool<PoolBase>;

		static SmartReference<Object>& default_ref() requires (std::is_default_constructible_v<Object>)
		{
			static SmartReference<Object> def;
			if (!def.valid())
				def.init();
			return def;
		}

		size_t pool_idx = size_t(-1);

		const SmartReferenceLink* link() const { return static_cast<const SmartReferenceLink*>(this); }
		SmartReferenceLink* link() { return static_cast<SmartReferenceLink*>(this); }

		template<typename T>
		struct IsSmartReferenceWithSharedPoolBase : public std::false_type {};

		template<typename T>
		struct IsSmartReferenceWithSharedPoolBase<SmartReference<T>> : public std::bool_constant<shares_smart_pool_base<T, Object>> {};

		template<typename... Args>
		static constexpr bool valid_object_args()
		{
			if constexpr (sizeof...(Args) == 1)
			{
				using T = std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
				return !std::is_same_v<T, nullptr_t>
					&& !std::is_same_v<T, internal::RefInit>
					&& !std::is_same_v<T, internal::RefDefault>
					&& !std::is_same_v<T, Object>
					&& !std::is_same_v<T, SmartReference<Object>>
					&& !shares_smart_pool_base<T, Object>
					&& !IsSmartReferenceWithSharedPoolBase<T>::value;
			}
			else
				return sizeof...(Args) > 1;
		}

		template<typename... Args>
		using EnableObjectArgs = std::enable_if_t<valid_object_args<Args...>()>;

		template<typename T>
		using SharesPoolBase = std::enable_if_t<shares_smart_pool_base<T, Object>>;

		template<typename T>
		using PreventUnconstFrom = std::enable_if_t<!std::is_const_v<T> || std::is_const_v<Object>>;

		template<typename T>
		using PreventUnconstTo = std::enable_if_t<!std::is_const_v<Object> || std::is_const_v<T>>;

	public:
		SmartReference() {}

		SmartReference(nullptr_t) {}

		SmartReference(internal::RefInit) requires (std::is_default_constructible_v<Object>)
		{
			init();
		}
		
		SmartReference(internal::RefDefault) requires (std::is_default_constructible_v<Object>)
		{
			pool_idx = default_ref().pool_idx;
			increment();
		}

		SmartReference(const Object& obj)
		{
			init(obj);
		}
		
		SmartReference(Object&& obj)
		{
			init(std::move(obj));
		}

		template<typename... Args, typename = EnableObjectArgs<Args...>>
		SmartReference(Args&&... args)
		{
			init(Object(std::forward<Args>(args)...));
		}

		SmartReference(const SmartReference<Object>& other)
			: pool_idx(other.pool_idx)
		{
			if (other.valid())
				increment();
		}

		template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstFrom<T>>
		SmartReference(const SmartReference<T>& other)
			: pool_idx(other.pool_idx)
		{
			if (other.valid())
				increment();
		}

		SmartReference(SmartReference<Object>&& other) noexcept
			: pool_idx(other.pool_idx)
		{
			if (other.valid())
				replace(other);
		}

		template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstFrom<T>>
		SmartReference(SmartReference<T>&& other) noexcept
			: pool_idx(other.pool_idx)
		{
			if (other.valid())
				replace(other);
		}

		SmartReference<Object>& operator=(const SmartReference<Object>& other)
		{
			if (this != &other)
			{
				if (valid())
				{
					if (other.valid())
					{
						if (pool_idx != other.pool_idx)
						{
							decrement();
							pool_idx = other.pool_idx;
							increment();
						}
					}
					else
						decrement();
				}
				else
				{
					if (other.valid())
					{
						pool_idx = other.pool_idx;
						increment();
					}
				}
			}
			return *this;
		}

		template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstFrom<T>>
		SmartReference<Object>& operator=(const SmartReference<T>& other)
		{
			if (this != &other)
			{
				if (valid())
				{
					if (other.valid())
					{
						if (pool_idx != other.pool_idx)
						{
							decrement();
							pool_idx = other.pool_idx;
							increment();
						}
					}
					else
						decrement();
				}
				else
				{
					if (other.valid())
					{
						pool_idx = other.pool_idx;
						increment();
					}
				}
			}
			return *this;
		}

		SmartReference<Object>& operator=(SmartReference<Object>&& other) noexcept
		{
			if (this != &other)
			{
				if (valid())
				{
					if (other.valid())
					{
						if (pool_idx != other.pool_idx)
						{
							decrement();
							pool_idx = other.pool_idx;
							replace(other);
						}
						else
							other.decrement();
					}
					else
						decrement();
				}
				else
				{
					if (other.valid())
					{
						pool_idx = other.pool_idx;
						replace(other);
					}
				}
			}
			return *this;
		}

		template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstFrom<T>>
		SmartReference<Object>& operator=(SmartReference<T>&& other) noexcept
		{
			if (this != &other)
			{
				if (valid())
				{
					if (other.valid())
					{
						if (pool_idx != other.pool_idx)
						{
							decrement();
							pool_idx = other.pool_idx;
							replace(other);
						}
						else
							other.decrement();
					}
					else
						decrement();
				}
				else
				{
					if (other.valid())
					{
						pool_idx = other.pool_idx;
						replace(other);
					}
				}
			}
			return *this;
		}

		~SmartReference()
		{
			invalidate();
		}

		const Object& operator*() const
		{
			if (valid())
				return *static_cast<const Object*>(pool().objects[pool_idx].get());
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		Object& operator*()
		{
			if (valid())
				return *static_cast<Object*>(pool().objects[pool_idx].get());
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		const Object* operator->() const
		{
			if (valid())
				return static_cast<const Object*>(pool().objects[pool_idx].get());
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		Object* operator->()
		{
			if (valid())
				return static_cast<Object*>(pool().objects[pool_idx].get());
			else
				throw Error(ErrorCode::NULL_POINTER);
		}

		const PoolBase* base() const
		{
			if (valid())
				return pool().objects[pool_idx].get();
			else
				return nullptr;
		}

		PoolBase* base()
		{
			if (valid())
				return pool().objects[pool_idx].get();
			else
				return nullptr;
		}

		template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstTo<T>>
		SmartReference<const T> as() const
		{
			return SmartReference<const T>(*this);
		}

		template<typename T, typename = SharesPoolBase<T>, typename = PreventUnconstTo<T>>
		SmartReference<T> as()
		{
			return SmartReference<T>(*this);
		}

		operator bool() const
		{
			return valid();
		}
		
		bool valid() const
		{
			return next || (pool_idx < pool().reference_heads.size() ? pool().reference_heads[pool_idx] == link() : false);
		}

		void init() requires (std::is_default_constructible_v<Object>)
		{
			if (valid())
				decrement();

			pool_idx = pool().init_slot();
			pool().increment_references(pool_idx, link());
		}
		
		void init(const Object& obj)
		{
			if (valid())
				decrement();

			pool_idx = pool().init_slot(obj);
			increment();
		}

		void init(Object&& obj)
		{
			if (valid())
				decrement();

			pool_idx = pool().init_slot(std::move(obj));
			increment();
		}

		template<typename... Args, typename = EnableObjectArgs<Args...>>
		void init(Args&&... args)
		{
			if (valid())
				decrement();

			pool_idx = pool().init_slot(Object(std::forward<Args>(args)...));
			increment();
		}

		void clone()
		{
			if (valid())
			{
				Object copy = *pool().objects[pool_idx];
				decrement();
				pool_idx = pool().init_slot(std::move(copy));
				increment();
			}
		}
		
		SmartReference<Object> get_clone() const
		{
			SmartReference<Object> cloned;
			if (valid())
			{
				cloned.pool_idx = pool().init_slot(*pool().objects[pool_idx]);
				cloned.increment();
			}
			return cloned;
		}
		
		void mark_for_deletion() const
		{
			if (valid())
				pool().mark_for_deletion(pool_idx);
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		bool is_marked_for_deletion() const
		{
			if (valid())
				return pool().is_marked_for_deletion(pool_idx);
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		void unmark_for_deletion() const
		{
			if (valid())
				pool().unmark_for_deletion(pool_idx);
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		SmartReference<Object>& operator=(std::nullptr_t)
		{
			invalidate();
			return *this;
		}

		void invalidate()
		{
			if (valid())
				decrement();
		}

		size_t hash() const
		{
			return std::hash<size_t>{}(valid() ? pool_idx : size_t(-1));
		}

		bool operator==(const SmartReference<Object>& other) const
		{
			if (valid())
				return other.valid() && pool_idx == other.pool_idx;
			else
				return other.valid();
		}

	private:
		static internal::SmartReferencePool<PoolBase>& pool()
		{
			return internal::SmartReferencePool<PoolBase>::instance();
		}

		void increment()
		{
			pool().increment_references(pool_idx, link());
		}

		void decrement()
		{
			pool().decrement_references(pool_idx, link());
		}

		template<typename T, typename = SharesPoolBase<T>>
		void replace(SmartReference<T>& other)
		{
			pool().replace_references(pool_idx, other.link(), link());
		}
	};

	namespace internal
	{
		template<typename Object>
		inline void SmartReferencePool<Object>::clear()
		{
			if constexpr (std::is_default_constructible_v<Object>)
				SmartReference<Object>::default_ref().invalidate();

			clear_stack(unoccupied);
			marked_for_deletion.clear();
			for (SmartReferenceLink* reference_head : reference_heads)
			{
				while (reference_head)
				{
					reference_head->prev = nullptr;
					SmartReferenceLink* next = reference_head->next;
					reference_head->next = nullptr;
					reference_head = next;
				}
			}
			reference_heads.clear();
			objects.clear();
		}
	}
}

template<typename T>
struct std::hash<oly::SmartReference<T>>
{
	size_t operator()(const oly::SmartReference<T>& h) const
	{
		return h.hash();
	}
};
