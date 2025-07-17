#include "MaterialRef.h"

#include "core/base/Errors.h"

namespace oly::physics
{
	static internal::MaterialPool& material_pool = internal::MaterialPool::instance();

	MaterialRef::MaterialRef(bool init)
	{
		if (init)
			this->init();
	}

	MaterialRef::MaterialRef(const MaterialRef& other)
		: valid(other.valid), pool_idx(other.pool_idx)
	{
		material_pool.increment_references(pool_idx, this);
	}

	MaterialRef::MaterialRef(MaterialRef&& other) noexcept
		: valid(other.valid), pool_idx(other.pool_idx)
	{
		material_pool.swap_references(pool_idx, &other, this);
		other.valid = false;
	}

	MaterialRef& MaterialRef::operator=(const MaterialRef& other)
	{
		if (this != &other)
		{
			if (valid)
			{
				if (other.valid)
				{
					if (pool_idx != other.pool_idx)
					{
						material_pool.decrement_references(pool_idx, this);
						pool_idx = other.pool_idx;
						material_pool.increment_references(pool_idx, this);
					}
				}
				else
				{
					material_pool.decrement_references(pool_idx, this);
					valid = false;
				}
			}
			else
			{
				if (other.valid)
				{
					pool_idx = other.pool_idx;
					material_pool.increment_references(pool_idx, this);
					valid = true;
				}
			}
		}
		return *this;
	}

	MaterialRef& MaterialRef::operator=(MaterialRef&& other) noexcept
	{
		if (this != &other)
		{
			if (valid)
			{
				if (other.valid)
				{
					if (pool_idx != other.pool_idx)
					{
						material_pool.decrement_references(pool_idx, this);
						pool_idx = other.pool_idx;
						material_pool.swap_references(pool_idx, &other, this);
						other.valid = false;
					}
					else
					{
						material_pool.decrement_references(pool_idx, &other);
						other.valid = false;
					}
				}
				else
				{
					material_pool.decrement_references(pool_idx, this);
					valid = false;
				}
			}
			else
			{
				if (other.valid)
				{
					pool_idx = other.pool_idx;
					material_pool.swap_references(pool_idx, &other, this);
					valid = true;
					other.valid = false;
				}
			}
		}
		return *this;
	}

	MaterialRef::~MaterialRef()
	{
		invalidate();
	}

	const Material& MaterialRef::operator*() const
	{
		if (valid)
			return material_pool.materials[pool_idx];
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	Material& MaterialRef::operator*()
	{
		if (valid)
			return material_pool.materials[pool_idx];
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	const Material* MaterialRef::operator->() const
	{
		if (valid)
			return &material_pool.materials[pool_idx];
		else
			return nullptr;
	}

	Material* MaterialRef::operator->()
	{
		if (valid)
			return &material_pool.materials[pool_idx];
		else
			return nullptr;
	}

	MaterialRef::operator bool() const
	{
		return valid;
	}

	bool MaterialRef::is_valid() const
	{
		return valid;
	}

	void MaterialRef::init()
	{
		if (valid)
			material_pool.decrement_references(pool_idx, this);

		pool_idx = material_pool.init_slot();
		material_pool.increment_references(pool_idx, this);
		valid = true;
	}

	void MaterialRef::clone()
	{
		if (valid)
		{
			Material copy = **this;
			material_pool.decrement_references(pool_idx, this);
			pool_idx = material_pool.init_slot(copy);
			material_pool.increment_references(pool_idx, this);
		}
	}

	MaterialRef MaterialRef::get_clone() const
	{
		MaterialRef cloned;
		if (valid)
		{
			cloned.valid = true;
			cloned.pool_idx = material_pool.init_slot(**this);
			material_pool.increment_references(cloned.pool_idx, &cloned);
		}
		return cloned;
	}

	void MaterialRef::mark_for_deletion() const
	{
		if (valid)
			material_pool.mark_for_deletion(pool_idx);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	bool MaterialRef::is_marked_for_deletion() const
	{
		if (valid)
			return material_pool.is_marked_for_deletion(pool_idx);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void MaterialRef::unmark_for_deletion() const
	{
		if (valid)
			material_pool.unmark_for_deletion(pool_idx);
		else
			throw Error(ErrorCode::NULL_POINTER);
	}

	void MaterialRef::invalidate()
	{
		if (valid)
			material_pool.decrement_references(pool_idx, this);
		valid = false;
	}

	namespace internal
	{
		void MaterialPool::clean()
		{
			for (size_t idx : marked_for_deletion)
			{
				for (MaterialRef* mat : references[idx])
					mat->valid = false;
				references[idx].clear();
			}

			marked_for_deletion.clear();
		}

		size_t MaterialPool::init_slot(const Material& mat)
		{
			if (unoccupied.empty())
			{
				materials.push_back(mat);
				references.push_back({});
				return materials.size() - 1;
			}
			else
			{
				size_t next_slot = unoccupied.top();
				materials[next_slot] = mat;
				unoccupied.pop();
				return next_slot;
			}
		}

		void MaterialPool::mark_for_deletion(size_t idx)
		{
			marked_for_deletion.insert(idx);
		}

		void MaterialPool::unmark_for_deletion(size_t idx)
		{
			marked_for_deletion.erase(idx);
		}

		bool MaterialPool::is_marked_for_deletion(size_t idx) const
		{
			return marked_for_deletion.count(idx);
		}

		void MaterialPool::increment_references(size_t idx, MaterialRef* mat)
		{
			references[idx].insert(mat);
		}
		
		void MaterialPool::decrement_references(size_t idx, MaterialRef* mat)
		{
			references[idx].erase(mat);
			if (references[idx].empty())
			{
				marked_for_deletion.erase(idx);
				unoccupied.push(idx);
			}
		}

		void MaterialPool::swap_references(size_t idx, MaterialRef* existing, MaterialRef* with)
		{
			if (references[idx].erase(existing))
				references[idx].insert(with);
		}
	}
}
