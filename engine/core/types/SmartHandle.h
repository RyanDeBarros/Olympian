#pragma once

#include "core/base/Errors.h"

#include <stack>
#include <unordered_set>

namespace oly
{
	template<typename Object>
	struct SmartHandle;

	namespace internal
	{
		// LATER multi-threading and thead safety
		template<typename Object>
		class SmartHandlePool
		{
			std::vector<Object> objects;
			std::stack<size_t> unoccupied;
			std::unordered_set<size_t> marked_for_deletion;
			std::vector<SmartHandle<Object>*> reference_heads;

			SmartHandlePool() = default;
			SmartHandlePool(const SmartHandlePool<Object>&) = delete;
			SmartHandlePool(SmartHandlePool<Object>&&) = delete;
			~SmartHandlePool() { clean(); }

		public:
			static SmartHandlePool& instance()
			{
				static SmartHandlePool pool;
				return pool;
			}

			void clean();

		private:
			friend struct SmartHandle<Object>;

			size_t init_slot()
			{
				if (unoccupied.empty())
				{
					objects.push_back(Object());
					reference_heads.push_back(nullptr);
					return objects.size() - 1;
				}
				else
				{
					size_t next_slot = unoccupied.top();
					objects[next_slot] = {};
					unoccupied.pop();
					return next_slot;
				}
			}

			size_t init_slot(const Object& obj)
			{
				if (unoccupied.empty())
				{
					objects.push_back(obj);
					reference_heads.push_back(nullptr);
					return objects.size() - 1;
				}
				else
				{
					size_t next_slot = unoccupied.top();
					objects[next_slot] = obj;
					unoccupied.pop();
					return next_slot;
				}
			}

			size_t init_slot(Object&& obj)
			{
				if (unoccupied.empty())
				{
					objects.push_back(std::move(obj));
					reference_heads.push_back(nullptr);
					return objects.size() - 1;
				}
				else
				{
					size_t next_slot = unoccupied.top();
					objects[next_slot] = std::move(obj);
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
			
			void increment_references(size_t idx, SmartHandle<Object>* handle);
			void decrement_references(size_t idx, SmartHandle<Object>* handle);
			void replace_references(size_t idx, SmartHandle<Object>* existing, SmartHandle<Object>* with);
		};
	}

	template<typename Object>
	struct SmartHandle
	{
	private:
		struct _Init {};

	public:
		static inline const _Init INIT;

	private:
		friend class internal::SmartHandlePool<Object>;

		size_t pool_idx = size_t(-1);
		SmartHandle<Object>* prev_adj_reference = nullptr;
		SmartHandle<Object>* next_adj_reference = nullptr;

	public:
		SmartHandle() = default;

		SmartHandle(nullptr_t) {}

		SmartHandle(_Init)
		{
			init();
		}
		
		SmartHandle(const Object& obj)
		{
			init(obj);
		}
		
		SmartHandle(Object&& obj)
		{
			init(std::move(obj));
		}

		template<typename... Args, typename = std::enable_if_t<sizeof...(Args) != 1
			|| !same_as_any<Args, nullptr_t, _Init, Object, SmartHandle<Object>>>>
		SmartHandle(Args&&... args)
		{
			init(Object(std::forward<Args>(args)...));
		}

		SmartHandle(const SmartHandle<Object>& other)
			: pool_idx(other.pool_idx)
		{
			if (other.valid())
				pool().increment_references(pool_idx, this);
		}

		SmartHandle(SmartHandle<Object>&& other) noexcept
			: pool_idx(other.pool_idx)
		{
			if (other.valid())
				pool().replace_references(pool_idx, &other, this);
		}
		
		SmartHandle<Object>& operator=(const SmartHandle<Object>& other)
		{
			if (this != &other)
			{
				if (valid())
				{
					if (other.valid())
					{
						if (pool_idx != other.pool_idx)
						{
							pool().decrement_references(pool_idx, this);
							pool_idx = other.pool_idx;
							pool().increment_references(pool_idx, this);
						}
					}
					else
						pool().decrement_references(pool_idx, this);
				}
				else
				{
					if (other.valid())
					{
						pool_idx = other.pool_idx;
						pool().increment_references(pool_idx, this);
					}
				}
			}
			return *this;
		}
		
		SmartHandle<Object>& operator=(SmartHandle<Object>&& other) noexcept
		{
			if (this != &other)
			{
				if (valid())
				{
					if (other.valid())
					{
						if (pool_idx != other.pool_idx)
						{
							pool().decrement_references(pool_idx, this);
							pool_idx = other.pool_idx;
							pool().replace_references(pool_idx, &other, this);
						}
						else
							pool().decrement_references(pool_idx, &other);
					}
					else
						pool().decrement_references(pool_idx, this);
				}
				else
				{
					if (other.valid())
					{
						pool_idx = other.pool_idx;
						pool().replace_references(pool_idx, &other, this);
					}
				}
			}
			return *this;
		}
		
		~SmartHandle()
		{
			invalidate();
		}

		const Object& operator*() const
		{
			if (valid())
				return pool().objects[pool_idx];
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		Object& operator*()
		{
			if (valid())
				return pool().objects[pool_idx];
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		const Object* operator->() const
		{
			if (valid())
				return &pool().objects[pool_idx];
			else
				return nullptr;
		}
		
		Object* operator->()
		{
			if (valid())
				return &pool().objects[pool_idx];
			else
				return nullptr;
		}

		operator bool() const
		{
			return valid();
		}
		
		bool valid() const
		{
			if (next_adj_reference)
				return true;
			if (pool_idx < pool().reference_heads.size())
				return pool().reference_heads[pool_idx] == this;
			else
				return false;
		}

		void init()
		{
			if (valid())
				pool().decrement_references(pool_idx, this);

			pool_idx = pool().init_slot();
			pool().increment_references(pool_idx, this);
		}
		
		void init(const Object& obj)
		{
			if (valid())
				pool().decrement_references(pool_idx, this);

			pool_idx = pool().init_slot(obj);
			pool().increment_references(pool_idx, this);
		}

		void init(Object&& obj)
		{
			if (valid())
				pool().decrement_references(pool_idx, this);

			pool_idx = pool().init_slot(std::move(obj));
			pool().increment_references(pool_idx, this);
		}

		void clone()
		{
			if (valid())
			{
				Object copy = pool().objects[pool_idx];
				pool().decrement_references(pool_idx, this);
				pool_idx = pool().init_slot(copy);
				pool().increment_references(pool_idx, this);
			}
		}
		
		SmartHandle<Object> get_clone() const
		{
			SmartHandle<Object> cloned;
			if (valid())
			{
				cloned.pool_idx = pool().init_slot(pool().objects[pool_idx]);
				pool().increment_references(cloned.pool_idx, &cloned);
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
		
		SmartHandle<Object>& operator=(std::nullptr_t)
		{
			invalidate();
			return *this;
		}

		void invalidate()
		{
			if (valid())
				pool().decrement_references(pool_idx, this);
		}

		size_t hash() const
		{
			return std::hash<size_t>{}(valid() ? pool_idx : size_t(-1));
		}

		bool operator==(const SmartHandle<Object>& other) const
		{
			if (valid())
				return other.valid() && pool_idx == other.pool_idx;
			else
				return other.valid();
		}

		static internal::SmartHandlePool<Object>& pool()
		{
			return internal::SmartHandlePool<Object>::instance();
		}
	};

	namespace internal
	{
		template<typename Object>
		void SmartHandlePool<Object>::clean()
		{
			for (size_t idx : marked_for_deletion)
			{
				while (reference_heads[idx])
				{
					reference_heads[idx]->prev_adj_reference = nullptr;
					SmartHandle<Object>* next = reference_heads[idx]->next_adj_reference;
					reference_heads[idx]->next_adj_reference = nullptr;
					reference_heads[idx] = next;
				}
			}

			marked_for_deletion.clear();
		}

		template<typename Object>
		inline void SmartHandlePool<Object>::increment_references(size_t idx, SmartHandle<Object>* handle)
		{
			if (reference_heads[idx])
			{
				SmartHandle<Object>* next = reference_heads[idx]->next_adj_reference;
				reference_heads[idx]->next_adj_reference = handle;
				handle->prev_adj_reference = reference_heads[idx];
				if (next)
				{
					handle->next_adj_reference = next;
					next->prev_adj_reference = handle;
				}
				else
				{
					handle->next_adj_reference = reference_heads[idx];
					reference_heads[idx]->prev_adj_reference = handle;
				}
			}
			else
				reference_heads[idx] = handle;
		}

		template<typename Object>
		inline void SmartHandlePool<Object>::decrement_references(size_t idx, SmartHandle<Object>* handle)
		{
			SmartHandle<Object>* prev = handle->prev_adj_reference;
			SmartHandle<Object>* next = handle->next_adj_reference;
			
			if (next == prev)
			{
				if (next)
				{
					next->prev_adj_reference = nullptr;
					next->next_adj_reference = nullptr;
				}
			}
			else
			{
				if (next)
					next->prev_adj_reference = prev;
				if (prev)
					prev->next_adj_reference = next;
			}
			
			if (reference_heads[idx] == handle)
				reference_heads[idx] = next;
			
			handle->prev_adj_reference = nullptr;
			handle->next_adj_reference = nullptr;

			if (!reference_heads[idx])
			{
				marked_for_deletion.erase(idx);
				unoccupied.push(idx);
			}
		}

		template<typename Object>
		inline void SmartHandlePool<Object>::replace_references(size_t idx, SmartHandle<Object>* existing, SmartHandle<Object>* with)
		{
			SmartHandle<Object>* prev = existing->prev_adj_reference;
			SmartHandle<Object>* next = existing->next_adj_reference;

			existing->prev_adj_reference = nullptr;
			existing->next_adj_reference = nullptr;
			
			with->prev_adj_reference = prev;
			with->next_adj_reference = next;
			
			if (prev)
				prev->next_adj_reference = with;
			if (next)
				next->prev_adj_reference = with;
			
			if (reference_heads[idx] == existing)
				reference_heads[idx] = with;
		}
	}
}

template<typename T>
struct std::hash<oly::SmartHandle<T>>
{
	size_t operator()(const oly::SmartHandle<T>& h) const
	{
		return h.hash();
	}
};
