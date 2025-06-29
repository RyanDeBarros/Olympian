#include "CollisionTree.h"

#include "core/base/Assert.h"
#include "core/containers/DoubleBuffer.h"

#include <stack>

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
			node->insert(*this);
	}

	Collider::Collider(Collider&& other) noexcept
		: tree(other.tree), dirty(other.dirty), quad_wrap(other.quad_wrap)
	{
		replace_in_node(std::move(other));
	}

	Collider::~Collider()
	{
		if (node)
			node->remove(*this);
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
						node->remove(*this);
					else
						other.node->insert(*this);
				}
			}
			else
			{
				if (node)
					node->remove(*this);

				tree = other.tree;
				node = other.node;

				if (node)
					node->insert(*this);
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
				node->remove(*this);

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
			node->colliders.replace(&other, this);
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
		: tree(tree), subnodes(tree.degree.x* tree.degree.y)
	{
	}

	std::unique_ptr<CollisionNode> CollisionNode::instantiate(CollisionTree& tree)
	{
		return std::unique_ptr<CollisionNode>(new CollisionNode(tree));
	}

	void CollisionNode::insert(const Collider& collider)
	{
		if (collider.quad_wrap.strict_inside(bounds))
		{
			colliders.insert(&collider);
			collider.node = this;
		}
	}

	void CollisionNode::remove(const Collider& collider)
	{
		if (colliders.erase(&collider))
			collider.node = nullptr;
	}

	void CollisionNode::update(const Collider& collider)
	{
		colliders.erase(&collider);
		collider.node = nullptr;
		insert_upwards(collider);
	}

	void CollisionNode::insert_upwards(const Collider& collider)
	{
		if (collider.quad_wrap.strict_inside(bounds))
		{
			if (colliders.insert(&collider))
				collider.node = this;
		}
		else if (parent)
			parent->insert_upwards(collider);
	}

	void CollisionNode::subdivide()
	{
		if (colliders.size() >= tree.cell_capacity)
		{
			size_t i = 0;
			while (i < colliders.size())
			{
				unsigned int x, y;
				if (subnode_coordinates(colliders[i]->quad_wrap, x, y))
				{
					std::unique_ptr<CollisionNode>& sub = subnode(x, y);
					if (!sub.get())
					{
						sub = instantiate(tree);
						sub->parent = this;
						sub->bounds = subdivision(x, y);
					}
					sub->colliders.insert(colliders[i]);
					colliders[i]->node = sub.get();
					colliders.remove(i);
					if (colliders.size() < tree.cell_capacity)
						return;
				}
				else
					++i;
			}
		}
	}

	bool CollisionNode::subnode_coordinates(const math::Rect2D& b, unsigned int& x, unsigned int& y) const
	{
		static const auto coord = [](unsigned int degree, float val, float rel, float len) {
			return (float)degree * (val - rel) / len;
			};

		float x1 = coord(tree.degree.x, b.x1, bounds.x1, bounds.width());
		if (near_zero(x1 - trunc(x1)))
			return false;

		float x2 = coord(tree.degree.x, b.x2, bounds.x1, bounds.width());
		if (near_zero(x2 - trunc(x2)))
			return false;

		if ((unsigned int)x1 != (unsigned int)x2)
			return false;

		float y1 = coord(tree.degree.y, b.y1, bounds.y1, bounds.height());
		if (near_zero(y1 - trunc(y1)))
			return false;

		float y2 = coord(tree.degree.y, b.y2, bounds.y1, bounds.height());
		if (near_zero(y2 - trunc(y2)))
			return false;

		if ((unsigned int)y1 != (unsigned int)y2)
			return false;

		x = (unsigned int)x1;
		y = (unsigned int)y1;
		return true;
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
		flush_update_colliders();
		flush_insert_downward();
		flush_remove_upward();
	}

	void CollisionTree::flush_update_colliders() const
	{
		// BFS

		DoubleBuffer<CollisionNode*> nodes;
		nodes.back.push_back(root.get());
		nodes.swap();
		while (!nodes.front.empty())
		{
			for (CollisionNode* node : nodes.front)
			{
				for (const Collider* collider : node->colliders)
					collider->flush();

				for (std::unique_ptr<CollisionNode>& subnode : root->subnodes)
				{
					if (subnode.get())
						nodes.back.push_back(subnode.get());
				}
			}
			nodes.front.clear();
			nodes.swap();
		}
	}

	void CollisionTree::flush_insert_downward() const
	{
		// BFS

		DoubleBuffer<CollisionNode*> nodes;
		nodes.back.push_back(root.get());
		nodes.swap();
		while (!nodes.front.empty())
		{
			for (CollisionNode* node : nodes.front)
			{
				node->subdivide();
				
				for (std::unique_ptr<CollisionNode>& subnode : root->subnodes)
				{
					if (subnode.get())
						nodes.back.push_back(subnode.get());
				}
			}
			nodes.front.clear();
			nodes.swap();
		}
	}

	void CollisionTree::flush_remove_upward() const
	{
		// DFS post-order

		struct Indexer
		{
			CollisionNode* node = nullptr;
			size_t index_in_parent = 0;
			bool visited_children = false;

			Indexer(CollisionNode* node, size_t index_in_parent, bool visited_children) : node(node), index_in_parent(index_in_parent), visited_children(visited_children) {}
		};
		std::stack<Indexer> stack;
		stack.emplace(root.get(), 0, false);
		while (!stack.empty())
		{
			Indexer indexer = stack.top();
			stack.pop();

			if (indexer.visited_children)
			{
				if (CollisionNode* parent = indexer.node->parent) // non-root
				{
					// due to traversal, it is guaranteed that node has no subnodes
					if (parent->colliders.size() < cell_capacity)
					{
						// transfer some colliders over to parent from node
						const size_t transfers = std::min(cell_capacity - parent->colliders.size(), indexer.node->colliders.size());
						for (size_t _ = 0; _ < transfers; ++_)
						{
							const Collider* collider = indexer.node->colliders.pop();
							parent->colliders.insert(collider);
							collider->node = parent;
						}
					}
					if (indexer.node->colliders.empty())
					{
						// node is an empty subnode
						parent->subnodes[indexer.index_in_parent].reset();
					}
				}
			}
			else
			{
				stack.emplace(indexer.node, indexer.index_in_parent, true);
				auto& subnodes = indexer.node->subnodes;
				for (size_t i = 0; i < subnodes.size(); ++i)
				{
					size_t idx = subnodes.size() - 1 - i;
					if (subnodes[idx].get())
						stack.emplace(subnodes[idx].get(), idx, false);
				}
			}
		}
	}

	void CollisionTree::insert(const Collider& collider)
	{
		root->insert(collider);
	}
}
