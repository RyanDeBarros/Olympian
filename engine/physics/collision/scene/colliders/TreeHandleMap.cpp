#include "TreeHandleMap.h"

#include "core/context/Collision.h"
#include "physics/collision/scene/dispatch/CollisionDispatcher.h"
#include "physics/collision/scene/colliders/Collider.h"

namespace oly::col2d::internal
{
	TreeHandleMap::TreeHandleMap(Collider& collider, const TreeHandleMap& other)
		: collider(collider), handles(other.handles)
	{
		for (auto& [tree, node] : handles)
			if (node)
				node->set_colliders().insert(&collider);
	}

	TreeHandleMap::TreeHandleMap(Collider& collider, TreeHandleMap&& other) noexcept
		: collider(collider), handles(std::move(other.handles))
	{
		for (auto& [tree, node] : handles)
			if (node)
				node->set_colliders().replace(&other.collider, &collider);
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
						it->value->set_colliders().insert(&collider);
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
					node->set_colliders().replace(&other.collider, &collider);
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
				tree->root->set_colliders().insert(&collider);
				node = tree->root.get();
			}
		}
	}

	void TreeHandleMap::attach(const CollisionTree& tree)
	{
		if (!handles.count(&tree))
		{
			tree.root->set_colliders().insert(&collider);
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
				it->value->set_colliders().erase(&collider);
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
				node->set_colliders().erase(&collider);
		handles.clear();
	}
}