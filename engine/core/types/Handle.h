#pragma once

#include "core/base/Errors.h"

#include <stack>
#include <unordered_set>

namespace oly
{
	template<typename Object>
	struct Handle;

	namespace internal
	{
		// LATER multi-threading and thead safety
		// TODO replace many uses of shared_ptr with Handle system.
		template<typename Object>
		class HandlePool
		{
			std::vector<Object> objects;
			std::stack<size_t> unoccupied;
			std::unordered_set<size_t> marked_for_deletion;
			std::vector<Handle<Object>*> reference_heads;

			HandlePool() = default;
			HandlePool(const HandlePool<Object>&) = delete;
			HandlePool(HandlePool<Object>&&) = delete;
			~HandlePool() { clean(); }

		public:
			static HandlePool& instance()
			{
				static HandlePool pool;
				return pool;
			}

			void clean();

		private:
			friend struct Handle<Object>;

			size_t init_slot(const Object& obj = {})
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
			
			void increment_references(size_t idx, Handle<Object>* handle);
			void decrement_references(size_t idx, Handle<Object>* handle);
			void swap_references(size_t idx, Handle<Object>* existing, Handle<Object>* with);
		};
	}

	template<typename Object>
	struct Handle
	{
	private:
		friend class internal::HandlePool<Object>;

		size_t pool_idx = size_t(-1);
		Handle<Object>* prev_adj_reference = nullptr;
		Handle<Object>* next_adj_reference = nullptr;

	public:
		Handle(bool init = false)
		{
			if (init)
				this->init();
		}

		Handle(const Handle<Object>& other)
			: pool_idx(other.pool_idx)
		{
			pool().increment_references(pool_idx, this);
		}

		Handle(Handle<Object>&& other) noexcept
			: pool_idx(other.pool_idx)
		{
			pool().swap_references(pool_idx, &other, this);
		}
		
		Handle<Object>& operator=(const Handle<Object>& other)
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
		
		Handle<Object>& operator=(Handle<Object>&& other) noexcept
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
							pool().swap_references(pool_idx, &other, this);
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
						pool().swap_references(pool_idx, &other, this);
					}
				}
			}
			return *this;
		}
		
		~Handle()
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
		
		Handle<Object> get_clone() const
		{
			Handle<Object> cloned;
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
		
		void invalidate()
		{
			if (valid())
				pool().decrement_references(pool_idx, this);
		}

		static internal::HandlePool<Object>& pool()
		{
			return internal::HandlePool<Object>::instance();
		}
	};

	namespace internal
	{
		template<typename Object>
		void HandlePool<Object>::clean()
		{
			for (size_t idx : marked_for_deletion)
			{
				while (reference_heads[idx])
				{
					reference_heads[idx]->prev_adj_reference = nullptr;
					Handle<Object>* next = reference_heads[idx]->next_adj_reference;
					reference_heads[idx]->next_adj_reference = nullptr;
					reference_heads[idx] = next;
				}
			}

			marked_for_deletion.clear();
		}

		template<typename Object>
		inline void HandlePool<Object>::increment_references(size_t idx, Handle<Object>* handle)
		{
			if (reference_heads[idx])
			{
				Handle<Object>* next = reference_heads[idx]->next_adj_reference;
				reference_heads[idx]->next_adj_reference = handle;
				handle->prev_adj_reference = reference_heads[idx];
				handle->next_adj_reference = next;
				if (next)
					next->prev_adj_reference = handle;
			}
			else
				reference_heads[idx] = handle;
		}

		template<typename Object>
		inline void HandlePool<Object>::decrement_references(size_t idx, Handle<Object>* handle)
		{
			Handle<Object>* prev = handle->prev_adj_reference;
			Handle<Object>* next = handle->next_adj_reference;
			
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
		inline void HandlePool<Object>::swap_references(size_t idx, Handle<Object>* existing, Handle<Object>* with)
		{
			Handle<Object>* prev = existing->prev_adj_reference;
			Handle<Object>* next = existing->next_adj_reference;

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
