#include "TreeHandleMap.h"

#include "core/context/Collision.h"

#include "physics/collision/scene/colliders/Collider.h"

#include "physics/collision/scene/dispatch/CollisionDispatcher.h"

namespace oly::col2d::internal
{
    TreeHandleMap::TreeHandleMap(Collider& collider)
        : collider_(collider)
    {
    }

	TreeHandleMap::TreeHandleMap(Collider& collider, const TreeHandleMap& o)
		: collider_(collider), map_(o.map_)
	{
		for (auto& [tree, node] : map_)
			if (node)
				node->set_colliders().insert(&collider_);
	}

	TreeHandleMap::TreeHandleMap(Collider& collider, TreeHandleMap&& o) noexcept
		: collider_(collider), map_(std::move(o.map_))
	{
		for (auto& [tree, node] : map_)
			if (node)
				node->set_colliders().replace(&o.collider_, &collider_);
	}

	TreeHandleMap::~TreeHandleMap()
	{
		clear();
	}

	TreeHandleMap& TreeHandleMap::operator=(const TreeHandleMap& o)
	{
		if (this != &o)
		{
			for (auto it = map_.begin(); it != map_.end(); )
			{
				if (o.map_.contains(it->key))
					++it;
				else
					it = map_.erase(it);
			}

			for (auto it = o.map_.begin(); it != o.map_.end(); ++it)
			{
				if (!map_.contains(it->key))
				{
					if (it->value)
						it->value->set_colliders().insert(&collider_);

					map_.assign(it->key, it->value);
				}
			}
		}
		return *this;
	}

	TreeHandleMap& TreeHandleMap::operator=(TreeHandleMap&& o) noexcept
	{
		if (this != &o)
		{
			clear();
			map_ = std::move(o.map_);
			for (auto& [tree, node] : map_)
				if (node)
					node->set_colliders().replace(&o.collider_, &collider_);
		}
		return *this;
	}

	void TreeHandleMap::flush() const
	{
		for (auto& [tree, node] : map_)
		{
			if (node)
				node->update(collider_, node);
			else
			{
				tree->root_->set_colliders().insert(&collider_);
				node = tree->root_.get();
			}
		}
	}

	void TreeHandleMap::attach(const CollisionTree& tree)
	{
		if (!map_.contains(&tree))
		{
			tree.root_->set_colliders().insert(&collider_);
			map_.assign(&tree, tree.root_.get());
		}
	}

	void TreeHandleMap::attach(size_t context_tree_index)
	{
		attach(CollisionDispatcher::instance().get_tree(context_tree_index));
	}

	void TreeHandleMap::detach(const CollisionTree& tree)
	{
		auto it = map_.find(&tree);
		if (it != map_.end())
		{
			if (it->value)
				it->value->set_colliders().erase(&collider_);
			map_.erase(it);
		}
	}

	void TreeHandleMap::detach(size_t context_tree_index)
	{
		detach(CollisionDispatcher::instance().get_tree(context_tree_index));
	}

    bool TreeHandleMap::is_attached(const CollisionTree& tree) const
    {
        return map_.contains(&tree);
    }

	bool TreeHandleMap::is_attached(size_t context_tree_index) const
	{
		return is_attached(CollisionDispatcher::instance().get_tree(context_tree_index));
	}

	void TreeHandleMap::clear()
	{
		for (auto& [tree, node] : map_)
			if (node)
				node->set_colliders().erase(&collider_);
		map_.clear();
	}

    size_t TreeHandleMap::size() const
    {
        return map_.size();
    }
}