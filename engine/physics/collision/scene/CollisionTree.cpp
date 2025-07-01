#include "CollisionTree.h"

#include "core/base/Assert.h"

#include <stack>

namespace oly::col2d
{
	Collider::Collider(const Collider& other)
		: obj(other.obj), tree(other.tree), node(other.node), dirty(other.dirty), quad_wrap(other.quad_wrap)
	{
		if (node)
			node->insert(*this);
	}

	Collider::Collider(Collider&& other) noexcept
		: obj(std::move(other.obj)), tree(other.tree), dirty(other.dirty), quad_wrap(other.quad_wrap)
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
			obj = other.obj;

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
			obj = std::move(other.obj);

			if (node)
				node->remove(*this);

			tree = other.tree;
			dirty = other.dirty;
			quad_wrap = other.quad_wrap;
			replace_in_node(std::move(other));
		}
		return *this;
	}

	void Collider::set_tree(CollisionTree* new_tree)
	{
		if (tree != new_tree)
		{
			if (node)
			{
				node->remove(*this);
				node = nullptr;
			}
			tree = new_tree;
		}
	}

	void Collider::unset_tree()
	{
		if (node)
		{
			node->remove(*this);
			node = nullptr;
		}
		tree = nullptr;
	}

	void Collider::replace_in_node(Collider&& other) noexcept
	{
		node = other.node;
		if (node)
		{
			node->colliders.replace(other.cref(), cref());
			other.node = nullptr;
		}
	}

	void Collider::flush() const
	{
		if (!is_dirty())
			return;

		dirty = false;
		internal::lut_flush(obj);

		if (node)
			node->update(*this);
		else
			tree->root->insert(*this);
	}

	CollisionNode::CollisionNode(CollisionTree& tree)
		: tree(tree), subnodes(tree.degree.x* tree.degree.y)
	{
	}

	CollisionNode::~CollisionNode()
	{
		for (const ConstSoftReference<Collider>& collider : colliders)
			if (auto c = collider.get())
				c->node = nullptr;
	}

	std::unique_ptr<CollisionNode> CollisionNode::instantiate(CollisionTree& tree)
	{
		return std::unique_ptr<CollisionNode>(new CollisionNode(tree));
	}

	void CollisionNode::insert(const Collider& collider)
	{
		colliders.insert(collider.cref());
		collider.node = this;
	}

	void CollisionNode::remove(const Collider& collider)
	{
		if (colliders.erase(collider.cref()))
			collider.node = nullptr;
	}

	void CollisionNode::update(const Collider& collider)
	{
		colliders.erase(collider.cref());
		collider.node = nullptr;
		insert_upwards(collider);
	}

	void CollisionNode::insert_upwards(const Collider& collider)
	{
		if (collider.quad_wrap.strict_inside(bounds))
		{
			if (colliders.insert(collider.cref()))
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
					std::unique_ptr<CollisionNode>& sub = subnodes[idx(x, y)];
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

	size_t CollisionNode::idx(unsigned int x, unsigned int y) const
	{
		return size_t(y) * tree.degree.x + x;
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

	CollisionTree::~CollisionTree()
	{
		BFSIterator it(root.get());
		while (CollisionNode* node = it.next())
		{
			for (const ConstSoftReference<Collider>& collider : node->colliders)
				if (auto c = collider.get())
					c->tree = nullptr;
		}
	}

	void CollisionTree::flush() const
	{
		flush_update_colliders();
		flush_insert_downward();
		flush_remove_upward();
	}

	void CollisionTree::flush_update_colliders() const
	{
		BFSIterator it(root.get());
		while (CollisionNode* node = it.next())
		{
			size_t i = 0;
			while (i < node->colliders.size())
			{
				const ConstSoftReference<Collider>& collider = node->colliders[i];
				if (auto c = collider.get())
				{
					c->flush();
					++i;
				}
				else
					node->colliders.remove(i);
			}
		}
	}

	void CollisionTree::flush_insert_downward() const
	{
		// Sensitive BFS
		std::queue<CollisionNode*> nodes;
		nodes.push(root.get());
		while (!nodes.empty())
		{
			CollisionNode* node = nodes.front();
			nodes.pop();

			node->subdivide();

			for (std::unique_ptr<CollisionNode>& subnode : node->subnodes)
			{
				if (subnode.get())
					nodes.push(subnode.get());
			}
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
							ConstSoftReference<Collider> collider = indexer.node->colliders.pop();
							if (auto c = collider.get())
							{
								parent->colliders.insert(collider);
								c->node = parent;
							}
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

	CollisionNode* CollisionTree::BFSIterator::next()
	{
		if (nodes.empty())
			return nullptr;

		CollisionNode* node = nodes.front();
		nodes.pop();

		for (const auto& subnode : node->subnodes)
			if (subnode.get())
				nodes.push(subnode.get());

		return node;
	}

	void CollisionTree::BFSColliderIterator::set(const BFSColliderIterator& other)
	{
		i = other.i;
		nodes.push(other.nodes.front());
	}

	ConstSoftReference<Collider> CollisionTree::BFSColliderIterator::next()
	{
		while (!nodes.empty())
		{
			const CollisionNode* node = nodes.front();
			if (i < node->colliders.size())
				return node->colliders[i++];

			i = 0;
			nodes.pop();
			for (const auto& subnode : node->subnodes)
				if (subnode.get() && subnode->bounds.overlaps(bounds))
					nodes.push(subnode.get());
		}
		return nullptr;
	}

	CollisionTree::PairIterator::PairIterator(CollisionNode* node, const math::Rect2D bounds)
		: first(node, bounds), second(bounds)
	{
		increment_current();
	}

	void CollisionTree::PairIterator::increment_current()
	{
		if (second.done())
		{
			current.first = first.next();
			if (!first.done())
			{
				second.set(first);
				current.second = second.next();
			}
			else
				current.first = current.second = nullptr;
		}
		else
			current.second = second.next();
	}

	CollisionTree::PairIterator::ColliderPtrPair CollisionTree::PairIterator::next()
	{
		ColliderPtrPair og = current;
		increment_current();
		return og;
	}
}
