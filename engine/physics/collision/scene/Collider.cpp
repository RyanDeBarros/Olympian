#include "Collider.h"

#include "core/base/Context.h"

namespace oly::col2d
{
	namespace internal
	{
		TreeHandleMap::TreeHandleMap(Collider& collider, const TreeHandleMap& other)
			: collider(collider), handles(other.handles)
		{
			for (auto& [tree, node] : handles)
				if (node)
					node->colliders.insert(collider.cref());
		}

		TreeHandleMap::TreeHandleMap(Collider& collider, TreeHandleMap&& other) noexcept
			: collider(collider), handles(std::move(other.handles))
		{
			for (auto& [tree, node] : handles)
				if (node)
					node->colliders.replace(other.collider.cref(), collider.cref());
		}

		TreeHandleMap::~TreeHandleMap()
		{
			clear();
		}

		TreeHandleMap& TreeHandleMap::operator=(const TreeHandleMap& other)
		{
			if (this != &other)
			{
				for (auto it = handles.begin(); it != handles.end(); )
				{
					if (other.handles.count(it->key))
						++it;
					else
						it = handles.erase(it);
				}

				for (auto it = other.handles.begin(); it != other.handles.end(); ++it)
				{
					if (!handles.count(it->key))
					{
						if (it->value)
							it->value->colliders.insert(collider.cref());
						handles.insert(it->key, it->value);
					}
				}
			}
			return *this;
		}

		TreeHandleMap& TreeHandleMap::operator=(TreeHandleMap&& other) noexcept
		{
			if (this != &other)
			{
				clear();
				handles = std::move(other.handles);
				for (auto& [tree, node] : handles)
					if (node)
						node->colliders.replace(other.collider.cref(), collider.cref());
			}
			return *this;
		}

		void TreeHandleMap::flush() const
		{
			for (auto& [tree, node] : handles)
			{
				if (node)
					node->update(collider, node);
				else
				{
					tree->root->colliders.insert(collider.cref());
					node = tree->root.get();
				}
			}
		}

		void TreeHandleMap::attach(const CollisionTree& tree)
		{
			if (!handles.count(&tree))
			{
				tree.root->colliders.insert(collider.cref());
				handles.insert(&tree, tree.root.get());
			}
		}

		void TreeHandleMap::attach(size_t context_tree_index)
		{
			attach(context::collision_dispatcher().get_tree(context_tree_index));
		}

		void TreeHandleMap::detach(const CollisionTree& tree)
		{
			auto it = handles.find(&tree);
			if (it != handles.end())
			{
				if (it->value)
					it->value->colliders.erase(collider.cref());
				handles.erase(it);
			}
		}

		void TreeHandleMap::detach(size_t context_tree_index)
		{
			detach(context::collision_dispatcher().get_tree(context_tree_index));
		}

		bool TreeHandleMap::is_attached(size_t context_tree_index) const
		{
			return is_attached(context::collision_dispatcher().get_tree(context_tree_index));
		}

		void TreeHandleMap::clear()
		{
			for (auto& [tree, node] : handles)
				if (node)
					node->colliders.erase(collider.cref());
			handles.clear();
		}
	}

	Collider::Collider(const Collider& other)
		: obj(other.obj), handles(*this, other.handles), dirty(other.dirty), quad_wrap(other.quad_wrap)
	{
	}

	Collider::Collider(Collider&& other) noexcept
		: obj(std::move(other.obj)), handles(*this, std::move(other.handles)), dirty(other.dirty), quad_wrap(other.quad_wrap)
	{
	}

	Collider& Collider::operator=(const Collider& other)
	{
		if (this != &other)
		{
			obj = other.obj;
			dirty = other.dirty;
			quad_wrap = other.quad_wrap;
			handles = other.handles;
		}
		return *this;
	}

	Collider& Collider::operator=(Collider&& other) noexcept
	{
		if (this != &other)
		{
			obj = std::move(other.obj);
			dirty = other.dirty;
			quad_wrap = other.quad_wrap;
			handles = std::move(other.handles);
		}
		return *this;
	}

	void Collider::flush() const
	{
		if (!is_dirty())
			return;

		dirty = false;
		internal::lut_flush(obj);
		handles.flush();
	}
}
