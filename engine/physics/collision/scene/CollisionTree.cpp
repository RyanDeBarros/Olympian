#include "CollisionTree.h"

#include "core/base/Assert.h"

namespace oly::col2d
{
	CollisionHandle::CollisionHandle(const ICollider* collider, CollisionNode* node)
		: collider(collider), node(node)
	{
	}

	CollisionHandle::CollisionHandle(CollisionHandle&& other) noexcept
		: collider(other.collider), node(other.node)
	{
		other.collider = nullptr;
		other.node = nullptr;
	}
	
	CollisionHandle::~CollisionHandle()
	{
		if (node)
			node->remove_direct(collider);
	}

	CollisionHandle& CollisionHandle::operator=(CollisionHandle&& other) noexcept
	{
		if (this != &other)
		{
			if (node)
				node->remove_direct(collider);
			collider = other.collider;
			node = other.node;
			other.collider = nullptr;
			other.node = nullptr;
		}
		return *this;
	}

	CollisionNode::CollisionNode(const CollisionTree* tree)
		: tree(tree), subnodes(tree->degree.x * tree->degree.y)
	{
	}

	std::unique_ptr<CollisionNode> CollisionNode::instantiate(const CollisionTree* tree)
	{
		return std::unique_ptr<CollisionNode>(new CollisionNode(tree));
	}

	CollisionHandle CollisionNode::insert(const ICollider* collider)
	{
		if (collider)
		{
			math::Rect2D b = collider->quad_wrap();
			if (b.inside(bounds))
				return insert(*collider, b);
		}
		return {};
	}

	CollisionHandle CollisionNode::insert(const ICollider& collider, const math::Rect2D& b)
	{
		auto it = std::find(colliders.begin(), colliders.end(), &collider);
		if (it != colliders.end())
			return {};

		if (colliders.size() >= tree->cell_capacity)
		{
			unsigned int x, y;
			if (subnode_coordinates(b, x, y))
			{
				if (!subnode(x, y).get())
				{
					subnode(x, y) = instantiate(tree);
					subnode(x, y)->bounds = subdivision(x, y);
				}
				return subnode(x, y)->insert(collider, b);
			}
		}
		colliders.push_back(&collider);
		return CollisionHandle(&collider, this);
	}
	
	bool CollisionNode::subnode_coordinates(const math::Rect2D& b, unsigned int& x, unsigned int& y) const
	{
		x = (unsigned int)(tree->degree.x * (b.x1 - bounds.x1) / bounds.x1);
		if ((unsigned int)(tree->degree.x * (b.x2 - bounds.x1) / bounds.x1) == x)
		{
			y = (unsigned int)(tree->degree.y * (b.y1 - bounds.y1) / bounds.y1);
			if ((unsigned int)(tree->degree.y * (b.y2 - bounds.y1) / bounds.y1) == y)
				return true;
		}
		return false;
	}

	const std::unique_ptr<CollisionNode>& CollisionNode::subnode(unsigned int x, unsigned int y) const
	{
		return subnodes[y * tree->degree.x + x];
	}

	std::unique_ptr<CollisionNode>& CollisionNode::subnode(unsigned int x, unsigned int y)
	{
		return subnodes[y * tree->degree.x + x];
	}
	
	void CollisionNode::remove_direct(const ICollider* collider)
	{
		if (collider)
		{
			auto it = std::find(colliders.begin(), colliders.end(), collider);
			if (it != colliders.end())
				colliders.erase(it);
		}
	}

	bool CollisionNode::remove(const ICollider* collider)
	{
		if (collider)
		{
			math::Rect2D b = collider->quad_wrap();
			if (b.inside(bounds))
				return remove(*collider, b);
		}
		return false;
	}

	bool CollisionNode::remove(const ICollider& collider, const math::Rect2D& b)
	{
		auto it = std::find(colliders.begin(), colliders.end(), &collider);
		if (it != colliders.end())
		{
			colliders.erase(it);
			return true;
		}
		else
		{
			unsigned int x, y;
			if (subnode_coordinates(b, x, y))
			{
				if (subnode(x, y).get() && subnode(x, y)->remove(&collider))
					return true;
			}
		}
		return false;
	}

	math::Rect2D CollisionNode::subdivision(int x, int y) const
	{
		return {
			.x1 = glm::mix(bounds.x1, bounds.x2, float(x) * tree->inv_degree.x), .x2 = glm::mix(bounds.x1, bounds.x2, (float(x) + 1.0f) * tree->inv_degree.x),
			.y1 = glm::mix(bounds.y1, bounds.y2, float(y) * tree->inv_degree.y), .y2 = glm::mix(bounds.y1, bounds.y2, (float(y) + 1.0f) * tree->inv_degree.y)
		};
	}

	CollisionTree::CollisionTree(math::Rect2D bounds, glm::uvec2 degree, size_t cell_capacity)
		: degree(degree), inv_degree(1.0f / glm::vec2(degree)), cell_capacity(cell_capacity)
	{
		OLY_ASSERT(degree.x * degree.y >= 2);
		OLY_ASSERT(cell_capacity >= 2);
		root = CollisionNode::instantiate(this);
		root->bounds = bounds;
	}

	CollisionHandle CollisionTree::insert(const ICollider* collider)
	{
		return root->insert(collider);
	}
}
