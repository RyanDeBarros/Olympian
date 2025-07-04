#include "CollisionTree.h"

#include "physics/collision/scene/Collider.h"
#include "core/base/Assert.h"

#include <stack>

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

		void TreeHandleMap::attach(CollisionTree* tree)
		{
			if (!handles.count(tree))
			{
				tree->root->colliders.insert(collider.cref());
				handles.insert(tree, tree->root.get());
			}
		}

		void TreeHandleMap::detach(CollisionTree* tree)
		{
			auto it = handles.find(tree);
			if (it != handles.end())
			{
				if (it->value)
					it->value->colliders.erase(collider.cref());
				handles.erase(it);
			}
		}

		void TreeHandleMap::clear()
		{
			for (auto& [tree, node] : handles)
				if (node)
					node->colliders.erase(collider.cref());
			handles.clear();
		}

		CollisionNode::CollisionNode(const CollisionTree* tree, math::Rect2D bounds)
			: tree(tree), bounds(bounds), subnodes(tree->degree.x * tree->degree.y)
		{
		}

		CollisionNode::CollisionNode(const CollisionTree* tree, CollisionNode* parent, const CollisionNode& other)
			: tree(tree), bounds(other.bounds), parent(parent), subnodes(tree->degree.x * tree->degree.y)
		{
			for (const ConstSoftReference<Collider>& collider : other.colliders)
			{
				if (const Collider* c = collider.get())
				{
					colliders.insert(collider);
					c->handles.handles[tree] = this;
				}
			}

			for (size_t i = 0; i < other.subnodes.size(); ++i)
			{
				const std::unique_ptr<CollisionNode>& sub = other.subnodes[i];
				if (CollisionNode* subnode = sub.get())
					subnodes[i] = std::unique_ptr<CollisionNode>(new CollisionNode(tree, this, *subnode));
			}
		}

		void CollisionNode::assign_tree(const CollisionTree* new_tree)
		{
			for (const ConstSoftReference<Collider>& collider : colliders)
			{
				if (const Collider* c = collider.get())
				{
					c->handles.handles.get(tree) = nullptr;
					c->handles.handles[tree] = this;
				}
			}

			tree = new_tree;

			for (const std::unique_ptr<CollisionNode>& sub : subnodes)
				if (CollisionNode* subnode = sub.get())
					subnode->assign_tree(tree);
		}

		CollisionNode::~CollisionNode()
		{
			for (const ConstSoftReference<Collider>& collider : colliders)
				if (const Collider* c = collider.get())
					c->handles.handles.get(tree) = nullptr;
		}

		std::unique_ptr<CollisionNode> CollisionNode::instantiate(const CollisionTree* tree, math::Rect2D bounds)
		{
			return std::unique_ptr<CollisionNode>(new CollisionNode(tree, bounds));
		}

		void CollisionNode::update(const Collider& collider, CollisionNode*& node)
		{
			colliders.erase(collider.cref());
			node = nullptr;
			insert_upwards(collider, node);
		}

		void CollisionNode::insert_upwards(const Collider& collider, CollisionNode*& node)
		{
			if (collider.quad_wrap.strict_inside(bounds))
			{
				if (colliders.insert(collider.cref()))
					node = this;
			}
			else if (parent)
				parent->insert_upwards(collider, node);
		}

		void CollisionNode::subdivide()
		{
			if (colliders.size() >= tree->cell_capacity)
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
							sub = instantiate(tree, subdivision(x, y));
							sub->parent = this;
						}
						sub->colliders.insert(colliders[i]);
						colliders[i]->handles.handles.get(tree) = sub.get();
						colliders.remove(i);
						if (colliders.size() < tree->cell_capacity)
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

			float x1 = coord(tree->degree.x, b.x1, bounds.x1, bounds.width());
			if (near_zero(x1 - trunc(x1)))
				return false;

			float x2 = coord(tree->degree.x, b.x2, bounds.x1, bounds.width());
			if (near_zero(x2 - trunc(x2)))
				return false;

			if ((unsigned int)x1 != (unsigned int)x2)
				return false;

			float y1 = coord(tree->degree.y, b.y1, bounds.y1, bounds.height());
			if (near_zero(y1 - trunc(y1)))
				return false;

			float y2 = coord(tree->degree.y, b.y2, bounds.y1, bounds.height());
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
			return size_t(y) * tree->degree.x + x;
		}

		math::Rect2D CollisionNode::subdivision(int x, int y) const
		{
			return {
				.x1 = glm::mix(bounds.x1, bounds.x2, float(x) * tree->inv_degree.x), .x2 = glm::mix(bounds.x1, bounds.x2, (float(x) + 1.0f) * tree->inv_degree.x),
				.y1 = glm::mix(bounds.y1, bounds.y2, float(y) * tree->inv_degree.y), .y2 = glm::mix(bounds.y1, bounds.y2, (float(y) + 1.0f) * tree->inv_degree.y)
			};
		}
	}

	CollisionTree::CollisionTree(math::Rect2D bounds, glm::uvec2 degree, size_t cell_capacity)
		: degree(degree), inv_degree(1.0f / glm::vec2(degree)), cell_capacity(cell_capacity)
	{
		OLY_ASSERT(degree.x * degree.y >= 2);
		OLY_ASSERT(cell_capacity >= 2);
		root = internal::CollisionNode::instantiate(this, bounds);
	}

	CollisionTree::CollisionTree(const CollisionTree& other)
		: cell_capacity(other.cell_capacity), degree(other.degree), inv_degree(other.inv_degree)
	{
		root = std::unique_ptr<internal::CollisionNode>(new internal::CollisionNode(this, nullptr, *other.root));
	}

	CollisionTree::CollisionTree(CollisionTree&& other) noexcept
		: cell_capacity(other.cell_capacity), degree(other.degree), inv_degree(other.inv_degree)
	{
		root = std::move(other.root);
		root->assign_tree(this);
	}

	CollisionTree::~CollisionTree()
	{
		BFSIterator it(root.get());
		while (internal::CollisionNode* node = it.next())
		{
			for (const ConstSoftReference<Collider>& collider : node->colliders)
				if (const Collider* c = collider.get())
					c->handles.handles.erase(this);
		}
	}

	CollisionTree& CollisionTree::operator=(const CollisionTree& other)
	{
		if (this != &other)
		{
			cell_capacity = other.cell_capacity;
			degree = other.degree;
			inv_degree = other.inv_degree;
			root = std::unique_ptr<internal::CollisionNode>(new internal::CollisionNode(this, nullptr, *other.root));
		}
		return *this;
	}

	CollisionTree& CollisionTree::operator=(CollisionTree&& other) noexcept
	{
		if (this != &other)
		{
			cell_capacity = other.cell_capacity;
			degree = other.degree;
			inv_degree = other.inv_degree;
			root = std::move(other.root);
			root->assign_tree(this);
		}
		return *this;
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
		while (internal::CollisionNode* node = it.next())
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
		std::queue<internal::CollisionNode*> nodes;
		nodes.push(root.get());
		while (!nodes.empty())
		{
			internal::CollisionNode* node = nodes.front();
			nodes.pop();

			node->subdivide();

			for (std::unique_ptr<internal::CollisionNode>& subnode : node->subnodes)
				if (internal::CollisionNode* sub = subnode.get())
					nodes.push(sub);
		}
	}

	void CollisionTree::flush_remove_upward() const
	{
		// DFS post-order
		struct Indexer
		{
			internal::CollisionNode* node = nullptr;
			size_t index_in_parent = 0;
			bool visited_children = false;

			Indexer(internal::CollisionNode* node, size_t index_in_parent, bool visited_children) : node(node), index_in_parent(index_in_parent), visited_children(visited_children) {}
		};
		std::stack<Indexer> stack;
		stack.emplace(root.get(), 0, false);
		while (!stack.empty())
		{
			Indexer indexer = stack.top();
			stack.pop();

			if (indexer.visited_children)
			{
				if (internal::CollisionNode* parent = indexer.node->parent) // non-root
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
								c->handles.handles.get(this) = parent;
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

	CollisionTree::BFSColliderIterator CollisionTree::query(const Collider& collider) const
	{
		return BFSColliderIterator(root.get(), collider.quad_wrap);
	}

	CollisionTree::BFSColliderIterator CollisionTree::query(const math::Rect2D bounds) const
	{
		return BFSColliderIterator(root.get(), bounds);
	}

	internal::CollisionNode* CollisionTree::BFSIterator::next()
	{
		if (nodes.empty())
			return nullptr;

		internal::CollisionNode* node = nodes.front();
		nodes.pop();

		for (std::unique_ptr<internal::CollisionNode>& subnode : node->subnodes)
			if (internal::CollisionNode* sub = subnode.get())
				nodes.push(sub);

		return node;
	}

	CollisionTree::BFSColliderIterator::BFSColliderIterator(const internal::CollisionNode* root, const math::Rect2D bounds)
		: bounds(bounds)
	{
		nodes.push(root);
		increment_current();
	}

	void CollisionTree::BFSColliderIterator::set(const BFSColliderIterator& other)
	{
		i = other.i;
		nodes.push(other.nodes.front());
		current = other.current;
	}

	void CollisionTree::BFSColliderIterator::increment_current()
	{
		while (!nodes.empty())
		{
			const internal::CollisionNode* node = nodes.front();
			if (i < node->colliders.size())
			{
				current = node->colliders[i++];
				return;
			}

			i = 0;
			nodes.pop();
			for (const auto& subnode : node->subnodes)
				if (subnode.get() && subnode->bounds.overlaps(bounds))
					nodes.push(subnode.get());
		}
		current = nullptr;
	}

	ConstSoftReference<Collider> CollisionTree::BFSColliderIterator::next()
	{
		ConstSoftReference<Collider> og = current;
		increment_current();
		return og;
	}

	CollisionTree::PairIterator::PairIterator(internal::CollisionNode* node, const math::Rect2D bounds)
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
