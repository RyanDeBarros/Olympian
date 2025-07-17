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
		template<typename Object>
		class HandlePool
		{
			std::vector<Object> objects;
			std::stack<size_t> unoccupied;
			std::unordered_set<size_t> marked_for_deletion;
			std::vector<std::unordered_set<Handle<Object>*>> references;

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
					references.push_back({});
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
			
			void increment_references(size_t idx, Handle<Object>* handle)
			{
				references[idx].insert(handle);
			}
			
			void decrement_references(size_t idx, Handle<Object>* handle)
			{
				references[idx].erase(handle);
				if (references[idx].empty())
				{
					marked_for_deletion.erase(idx);
					unoccupied.push(idx);
				}
			}
			
			void swap_references(size_t idx, Handle<Object>* existing, Handle<Object>* with)
			{
				if (references[idx].erase(existing))
					references[idx].insert(with);
			}
		};
	}

	template<typename Object>
	struct Handle
	{
	private:
		friend class internal::HandlePool<Object>;

		bool valid = false;
		size_t pool_idx = size_t(-1);

	public:
		Handle(bool init = false)
		{
			if (init)
				this->init();
		}

		Handle(const Handle<Object>& other)
			: valid(other.valid), pool_idx(other.pool_idx)
		{
			pool().increment_references(pool_idx, this);
		}

		Handle(Handle<Object>&& other) noexcept
			: valid(other.valid), pool_idx(other.pool_idx)
		{
			pool().swap_references(pool_idx, &other, this);
			other.valid = false;
		}
		
		Handle<Object>& operator=(const Handle<Object>& other)
		{
			if (this != &other)
			{
				if (valid)
				{
					if (other.valid)
					{
						if (pool_idx != other.pool_idx)
						{
							pool().decrement_references(pool_idx, this);
							pool_idx = other.pool_idx;
							pool().increment_references(pool_idx, this);
						}
					}
					else
					{
						pool().decrement_references(pool_idx, this);
						valid = false;
					}
				}
				else
				{
					if (other.valid)
					{
						pool_idx = other.pool_idx;
						pool().increment_references(pool_idx, this);
						valid = true;
					}
				}
			}
			return *this;
		}
		
		Handle<Object>& operator=(Handle<Object>&& other) noexcept
		{
			if (this != &other)
			{
				if (valid)
				{
					if (other.valid)
					{
						if (pool_idx != other.pool_idx)
						{
							pool().decrement_references(pool_idx, this);
							pool_idx = other.pool_idx;
							pool().swap_references(pool_idx, &other, this);
							other.valid = false;
						}
						else
						{
							pool().decrement_references(pool_idx, &other);
							other.valid = false;
						}
					}
					else
					{
						pool().decrement_references(pool_idx, this);
						valid = false;
					}
				}
				else
				{
					if (other.valid)
					{
						pool_idx = other.pool_idx;
						pool().swap_references(pool_idx, &other, this);
						valid = true;
						other.valid = false;
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
			if (valid)
				return pool().objects[pool_idx];
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		Object& operator*()
		{
			if (valid)
				return pool().objects[pool_idx];
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		const Object* operator->() const
		{
			if (valid)
				return &pool().objects[pool_idx];
			else
				return nullptr;
		}
		
		Object* operator->()
		{
			if (valid)
				return &pool().objects[pool_idx];
			else
				return nullptr;
		}

		operator bool() const
		{
			return valid;
		}
		
		bool is_valid() const
		{
			return valid;
		}
		
		void init()
		{
			if (valid)
				pool().decrement_references(pool_idx, this);

			pool_idx = pool().init_slot();
			pool().increment_references(pool_idx, this);
			valid = true;
		}
		
		void clone()
		{
			if (valid)
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
			if (valid)
			{
				cloned.valid = true;
				cloned.pool_idx = pool().init_slot(pool().objects[pool_idx]);
				pool().increment_references(cloned.pool_idx, &cloned);
			}
			return cloned;
		}
		
		void mark_for_deletion() const
		{
			if (valid)
				pool().mark_for_deletion(pool_idx);
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		bool is_marked_for_deletion() const
		{
			if (valid)
				return pool().is_marked_for_deletion(pool_idx);
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		void unmark_for_deletion() const
		{
			if (valid)
				pool().unmark_for_deletion(pool_idx);
			else
				throw Error(ErrorCode::NULL_POINTER);
		}
		
		void invalidate()
		{
			if (valid)
				pool().decrement_references(pool_idx, this);
			valid = false;
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
				for (Handle<Object>* handle : references[idx])
					handle->valid = false;
				references[idx].clear();
			}

			marked_for_deletion.clear();
		}
	}
}
