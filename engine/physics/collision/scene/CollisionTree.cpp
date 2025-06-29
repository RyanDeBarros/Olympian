#include "CollisionTree.h"

#include "core/base/Assert.h"

namespace oly::col2d
{
	Collider::Collider(CollisionTree& tree)
		: tree(&tree)
	{
	}

	Collider::Collider(const Collider& other)
		: tree(other.tree), node(other.node), dirty(other.dirty), quad_wrap(other.quad_wrap)
	{
		if (node)
			node->insert_direct(*this);
	}

	Collider::Collider(Collider&& other) noexcept
		: tree(other.tree), dirty(other.dirty), quad_wrap(other.quad_wrap)
	{
		replace_in_node(std::move(other));
	}

	Collider::~Collider()
	{
		if (node)
			node->remove_direct(*this);
	}

	Collider& Collider::operator=(const Collider& other)
	{
		if (this != &other)
		{
			if (tree == other.tree)
			{
				if (node != other.node)
				{
					if (node)
						node->remove_direct(*this);
					else
						other.node->insert_direct(*this);
				}
			}
			else
			{
				if (node)
					node->remove_direct(*this);

				tree = other.tree;
				node = other.node;

				if (node)
					node->insert_direct(*this);
			}

			dirty = other.dirty;
			quad_wrap = other.quad_wrap;
		}
		return *this;
	}

	Collider& Collider::operator=(Collider&& other) noexcept
	{
		if (this != &other)
		{
			if (node)
				node->remove_direct(*this);
			
			tree = other.tree;
			dirty = other.dirty;
			quad_wrap = other.quad_wrap;
			replace_in_node(std::move(other));
		}
		return *this;
	}

	void Collider::replace_in_node(Collider&& other) noexcept
	{
		node = other.node;
		if (node)
		{
			auto& colliders = node->colliders;
			auto it = std::find(colliders.begin(), colliders.end(), &other);
			if (it != colliders.end())
			{
				*it = this;
				tree->colliders.erase(&other);
				tree->colliders.insert(this);
			}
			other.node = nullptr;
		}
	}

	void Collider::flush() const
	{
		if (!is_dirty())
			return;

		dirty = false;
		flush_impl();

		if (node)
			node->update(*this);
		else
			tree->insert(*this);
	}

	CollisionNode::CollisionNode(CollisionTree& tree)
		: tree(tree), subnodes(tree.degree.x * tree.degree.y)
	{
	}

	std::unique_ptr<CollisionNode> CollisionNode::instantiate(CollisionTree& tree)
	{
		return std::unique_ptr<CollisionNode>(new CollisionNode(tree));
	}

	void CollisionNode::insert(const Collider& collider)
	{
		if (collider.quad_wrap.inside(bounds) && !tree.colliders.count(&collider))
		{
			tree.colliders.insert(&collider);
			collider.node = _insert(&collider);
		}
	}

	CollisionNode* CollisionNode::_insert(const Collider* collider)
	{
		if (colliders.size() >= tree.cell_capacity)
		{
			unsigned int x, y;
			if (subnode_coordinates(collider->quad_wrap, x, y))
			{
				std::unique_ptr<CollisionNode>& sub = subnode(x, y);
				if (!sub.get())
				{
					sub = instantiate(tree);
					sub->parent = this;
					sub->bounds = subdivision(x, y);
				}
				return sub->_insert(collider);
			}
		}
		colliders.push_back(collider);
		return this;
	}

	void CollisionNode::insert_direct(const Collider& collider)
	{
		if (!tree.colliders.count(&collider))
		{
			colliders.push_back(&collider);
			tree.colliders.insert(&collider);
			collider.node = this;
		}
	}
	
	bool CollisionNode::remove(const Collider& collider)
	{
		if (collider.quad_wrap.inside(bounds) && _remove(&collider))
		{
			tree.colliders.erase(&collider);
			collider.node = nullptr;
			return true;
		}
		return false;
	}

	bool CollisionNode::_remove(const Collider* collider)
	{
		auto it = std::find(colliders.begin(), colliders.end(), collider);
		if (it != colliders.end())
		{
			colliders.erase(it);
			return true;
		}
		else
		{
			unsigned int x, y;
			if (subnode_coordinates(collider->quad_wrap, x, y))
			{
				if (subnode(x, y).get() && subnode(x, y)->_remove(collider))
					return true;
			}
		}
		return false;
	}

	void CollisionNode::remove_direct(const Collider& collider)
	{
		auto it = std::find(colliders.begin(), colliders.end(), &collider);
		if (it != colliders.end())
		{
			colliders.erase(it);
			tree.colliders.erase(&collider);
			collider.node = nullptr;
		}
	}

	void CollisionNode::update(const Collider& collider)
	{
		auto it = std::find(colliders.begin(), colliders.end(), &collider);
		if (it != colliders.end())
			colliders.erase(it);

		insert_upwards(collider);
	}

	void CollisionNode::insert_upwards(const Collider& collider)
	{
		if (collider.quad_wrap.inside(bounds))
			collider.node = _insert(&collider);
		else if (parent)
			parent->insert_upwards(collider);
		else
		{
			tree.colliders.erase(&collider);
			collider.node = nullptr;
		}
	}

	bool CollisionNode::subnode_coordinates(const math::Rect2D& b, unsigned int& x, unsigned int& y) const
	{
		x = (unsigned int)(tree.degree.x * (b.x1 - bounds.x1) / bounds.x1);
		if ((unsigned int)(tree.degree.x * (b.x2 - bounds.x1) / bounds.x1) == x)
		{
			y = (unsigned int)(tree.degree.y * (b.y1 - bounds.y1) / bounds.y1);
			if ((unsigned int)(tree.degree.y * (b.y2 - bounds.y1) / bounds.y1) == y)
				return true;
		}
		return false;
	}

	const std::unique_ptr<CollisionNode>& CollisionNode::subnode(unsigned int x, unsigned int y) const
	{
		return subnodes[y * tree.degree.x + x];
	}

	std::unique_ptr<CollisionNode>& CollisionNode::subnode(unsigned int x, unsigned int y)
	{
		return subnodes[y * tree.degree.x + x];
	}

	math::Rect2D CollisionNode::subdivision(int x, int y) const
	{
		return {
			.x1 = glm::mix(bounds.x1, bounds.x2, float(x) * tree.inv_degree.x), .x2 = glm::mix(bounds.x1, bounds.x2, (float(x) + 1.0f) * tree.inv_degree.x),
			.y1 = glm::mix(bounds.y1, bounds.y2, float(y) * tree.inv_degree.y), .y2 = glm::mix(bounds.y1, bounds.y2, (float(y) + 1.0f) * tree.inv_degree.y)
		};
	}

	CollisionTree::CollisionTree(math::Rect2D bounds, glm::uvec2 degree, size_t cell_capacity)
		: degree(degree), inv_degree(1.0f / glm::vec2(degree)), cell_capacity(cell_capacity)
	{
		OLY_ASSERT(degree.x * degree.y >= 2);
		OLY_ASSERT(cell_capacity >= 2);
		root = CollisionNode::instantiate(*this);
		root->bounds = bounds;
	}

	void CollisionTree::flush() const
	{
		for (const Collider* collider : colliders)
			collider->flush();
	}

	void CollisionTree::insert(const Collider& collider)
	{
		root->insert(collider);
	}
}
