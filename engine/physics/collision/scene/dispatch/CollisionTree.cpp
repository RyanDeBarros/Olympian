#include "CollisionTree.h"

#include "physics/collision/scene/colliders/Collider.h"
#include "core/base/Assert.h"

#include <stack>

namespace oly::col2d
{
	namespace internal
	{
		CollisionNode::CollisionNode(const CollisionTree* tree, math::Rect2D bounds)
			: tree_(tree)
            , bounds_(bounds)
            , subnodes_(tree->degree_.x * tree->degree_.y)
		{
		}

		CollisionNode::CollisionNode(const CollisionTree* tree, CollisionNode* parent, const CollisionNode& o)
			: tree_(tree)
            , bounds_(o.bounds_)
            , parent_(parent)
            , subnodes_(tree->degree_.x * tree->degree_.y)
		{
			for (const Collider* collider : o.get_colliders())
			{
				set_colliders().insert(collider);
				collider->handles.map_.assign(tree, this);
			}

			for (size_t i_ = 0; i_ < o.subnodes_.size(); ++i_)
			{
				const std::unique_ptr<CollisionNode>& sub = o.subnodes_[i_];
				if (CollisionNode* subnode = sub.get())
					subnodes_[i_] = std::unique_ptr<CollisionNode>(new CollisionNode(tree, this, *subnode));
			}
		}

		CollisionNode::~CollisionNode()
		{
			for (const Collider* collider : get_colliders())
				collider->handles.map_.erase(tree_);
		}

        imp::contiguous_set<const Collider*>& CollisionNode::set_colliders()
        {
            tree_->invalidate_iterators();
            return colliders_;
        }

        const imp::contiguous_set<const Collider*>& CollisionNode::get_colliders() const
        {
            return colliders_;
        }

		std::unique_ptr<CollisionNode> CollisionNode::instantiate(const CollisionTree* tree, math::Rect2D bounds)
		{
			return std::unique_ptr<CollisionNode>(new CollisionNode(tree, bounds));
		}

        void CollisionNode::assign_tree(const CollisionTree* new_tree)
        {
            for (const Collider* collider : get_colliders())
            {
                collider->handles.map_.erase(tree_);
                collider->handles.map_.assign(new_tree, this);
            }

            tree_ = new_tree;

            for (const std::unique_ptr<CollisionNode>& sub : subnodes_)
            {
                if (CollisionNode* subnode = sub.get())
                    subnode->assign_tree(tree_);
            }
        }

		void CollisionNode::update(const Collider& collider, CollisionNode*& node)
		{
			if (!collider.quad_wrap_.strict_inside(bounds_) && parent_)
			{
				set_colliders().erase(&collider);
				node = nullptr;
				parent_->insert_upwards(collider, node);
			}
		}

		void CollisionNode::insert_upwards(const Collider& collider, CollisionNode*& node)
		{
			if (collider.quad_wrap_.strict_inside(bounds_))
			{
				if (set_colliders().insert(&collider))
					node = this;
			}
			else if (parent_)
				parent_->insert_upwards(collider, node);
		}

		void CollisionNode::subdivide()
		{
			if (get_colliders().size() >= tree_->cell_capacity_)
			{
				size_t i_ = 0;
				while (i_ < get_colliders().size())
				{
					unsigned int x, y;
					if (subnode_coordinates(get_colliders()[i_]->quad_wrap_, x, y))
					{
						std::unique_ptr<CollisionNode>& sub = subnodes_[idx(x, y)];
						if (!sub.get())
						{
							sub = instantiate(tree_, subdivision(x, y));
							sub->parent_ = this;
						}
						sub->set_colliders().insert(get_colliders()[i_]);
                        if (auto ptr = get_colliders()[i_]->handles.map_.try_get(tree_))
                        {
                            *ptr = sub.get();
                            set_colliders().remove(i_);
                            if (get_colliders().size() < tree_->cell_capacity_)
                                return;
                        }
                        else
                            throw Error(ErrorCode::NullPointer);
					}
					else
						++i_;
				}
			}
		}

		bool CollisionNode::subnode_coordinates(math::Rect2D b, unsigned int& x, unsigned int& y) const
		{
			static const auto coord = [](unsigned int degree_, float val, float rel, float len) {
				return (float)degree_ * (val - rel) / len;
				};

			float x1 = coord(tree_->degree_.x, b.x1, bounds_.x1, bounds_.width());
			if (near_zero(x1 - trunc(x1)))
				return false;

			float x2 = coord(tree_->degree_.x, b.x2, bounds_.x1, bounds_.width());
			if (near_zero(x2 - trunc(x2)))
				return false;

			if ((unsigned int)x1 != (unsigned int)x2)
				return false;

			float y1 = coord(tree_->degree_.y, b.y1, bounds_.y1, bounds_.height());
			if (near_zero(y1 - trunc(y1)))
				return false;

			float y2 = coord(tree_->degree_.y, b.y2, bounds_.y1, bounds_.height());
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
			return size_t(y) * tree_->degree_.x + x;
		}

		math::Rect2D CollisionNode::subdivision(int x, int y) const
		{
			return {
				.x1 = glm::mix(bounds_.x1, bounds_.x2, float(x) * tree_->inv_degree_.x),
                .x2 = glm::mix(bounds_.x1, bounds_.x2, (float(x) + 1.0f) * tree_->inv_degree_.x),
				.y1 = glm::mix(bounds_.y1, bounds_.y2, float(y) * tree_->inv_degree_.y),
                .y2 = glm::mix(bounds_.y1, bounds_.y2, (float(y) + 1.0f) * tree_->inv_degree_.y)
			};
		}

		void CollisionNode::set_bounds(math::Rect2D b)
		{
			bounds_ = b;
            for (unsigned int x = 0; x < tree_->degree_.x; ++x)
            {
                for (unsigned int y = 0; y < tree_->degree_.y; ++y)
                {
					if (CollisionNode* subnode = subnodes_[idx(x, y)].get())
						subnode->set_bounds(subdivision(x, y));
                }
            }
		}
	}

    CollisionTree::BFSColliderIterator::BFSColliderIterator(const math::Rect2D bounds)
        : bounds_(bounds)
    {
    }

    CollisionTree::BFSColliderIterator::BFSColliderIterator(const CollisionTree& tree, const math::Rect2D bounds)
        : tree_(&tree)
        , bounds_(bounds)
    {
        tree.bfs_collider_iterators_.insert(this);
        nodes_.push(tree.root_.get());
        increment_current();
    }

    CollisionTree::BFSColliderIterator::BFSColliderIterator(const BFSColliderIterator& o)
        : tree_(o.tree_)
        , bounds_(o.bounds_)
        , nodes_(o.nodes_)
        , i_(o.i_)
        , current_(o.current_)
    {
        if (tree_)
            tree_->bfs_collider_iterators_.insert(this);
    }

    CollisionTree::BFSColliderIterator::BFSColliderIterator(BFSColliderIterator&& o)
        : tree_(o.tree_)
        , bounds_(o.bounds_)
        , nodes_(std::move(o.nodes_))
        , i_(o.i_)
        , current_(o.current_)
    {
        if (tree_)
        {
            tree_->bfs_collider_iterators_.erase(&o);
            o.tree_ = nullptr;
            tree_->bfs_collider_iterators_.insert(this);
        }
    }

    CollisionTree::BFSColliderIterator::~BFSColliderIterator()
    {
        if (tree_)
            tree_->bfs_collider_iterators_.erase(this);
    }

    CollisionTree::BFSColliderIterator& CollisionTree::BFSColliderIterator::operator=(const BFSColliderIterator& o)
    {
        if (this != &o)
        {
            if (tree_)
                tree_->bfs_collider_iterators_.erase(this);
            tree_ = o.tree_;
            if (tree_)
                tree_->bfs_collider_iterators_.insert(this);

            bounds_ = o.bounds_;
            nodes_ = o.nodes_;
            i_ = o.i_;
            current_ = o.current_;
        }
        return *this;
    }

    CollisionTree::BFSColliderIterator& CollisionTree::BFSColliderIterator::operator=(BFSColliderIterator&& o)
    {
        if (this != &o)
        {
            if (tree_)
                tree_->bfs_collider_iterators_.erase(this);
            tree_ = o.tree_;
            if (tree_)
            {
                tree_->bfs_collider_iterators_.erase(&o);
                o.tree_ = nullptr;
                tree_->bfs_collider_iterators_.insert(this);
            }

            bounds_ = o.bounds_;
            nodes_ = std::move(o.nodes_);
            i_ = o.i_;
            current_ = o.current_;
        }
        return *this;
    }

    bool CollisionTree::BFSColliderIterator::done() const
    {
        return !tree_ || !current_;
    }

    const Collider* CollisionTree::BFSColliderIterator::next()
    {
        assert_valid();
        const Collider* og = current_;
        increment_current();
        return og;
    }

    void CollisionTree::BFSColliderIterator::set(const BFSColliderIterator& o)
    {
        i_ = o.i_;
        nodes_.push(o.nodes_.front());
        current_ = o.current_;
    }

    void CollisionTree::BFSColliderIterator::increment_current()
    {
        while (!nodes_.empty())
        {
            const internal::CollisionNode* node = nodes_.front();
            if (i_ < node->get_colliders().size())
            {
                current_ = node->get_colliders()[i_++];
                return;
            }

            i_ = 0;
            nodes_.pop();
            for (const auto& subnode : node->subnodes_)
                if (subnode.get() && subnode->bounds_.overlaps(bounds_))
                    nodes_.push(subnode.get());
        }
        current_ = nullptr;
    }

    void CollisionTree::BFSColliderIterator::assert_valid() const
    {
        if (!tree_)
            throw Error(ErrorCode::InvalidIterator);
    }

    void CollisionTree::BFSColliderIterator::invalidate() const
    {
        tree_->bfs_collider_iterators_.erase(this);
        tree_ = nullptr;
    }

    CollisionTree::PairIterator::ColliderPtrPair::operator bool() const
    {
        return first && second;
    }

    CollisionTree::PairIterator::PairIterator(const CollisionTree& tree, const math::Rect2D bounds)
        : tree_(&tree)
        , first_(tree, bounds)
        , second_(bounds)
    {
        second_.tree_ = tree_;
        tree.bfs_collider_iterators_.insert(&second_);
        tree.pair_iterators_.insert(this);
        increment_current();
    }

    CollisionTree::PairIterator::PairIterator(const PairIterator& o)
        : tree_(o.tree_)
        , first_(o.first_)
        , second_(o.second_)
        , current_(o.current_)
    {
        if (tree_)
            tree_->pair_iterators_.insert(this);
    }

    CollisionTree::PairIterator::PairIterator(PairIterator&& o)
        : tree_(o.tree_)
        , first_(std::move(o.first_))
        , second_(std::move(o.second_))
        , current_(o.current_)
    {
        if (tree_)
        {
            tree_->pair_iterators_.erase(&o);
            o.tree_ = nullptr;
            tree_->pair_iterators_.insert(this);
        }
    }

    CollisionTree::PairIterator::~PairIterator()
    {
        if (tree_)
            tree_->pair_iterators_.erase(this);
    }

    CollisionTree::PairIterator& CollisionTree::PairIterator::operator=(const PairIterator& o)
    {
        if (this != &o)
        {
            if (tree_)
                tree_->pair_iterators_.erase(this);
            tree_ = o.tree_;
            if (tree_)
                tree_->pair_iterators_.insert(this);

            first_ = o.first_;
            second_ = o.second_;
            current_ = o.current_;
        }
        return *this;
    }

    CollisionTree::PairIterator& CollisionTree::PairIterator::operator=(PairIterator&& o)
    {
        if (this != &o)
        {
            if (tree_)
                tree_->pair_iterators_.erase(this);
            tree_ = o.tree_;
            if (tree_)
            {
                tree_->pair_iterators_.erase(&o);
                o.tree_ = nullptr;
                tree_->pair_iterators_.insert(this);
            }

            first_ = std::move(o.first_);
            second_ = std::move(o.second_);
            current_ = o.current_;
        }
        return *this;
    }

    bool CollisionTree::PairIterator::done() const
    {
        return !tree_ || !current_;
    }

    CollisionTree::PairIterator::ColliderPtrPair CollisionTree::PairIterator::next()
    {
        assert_valid();
        ColliderPtrPair og = current_;
        increment_current();
        return og;
    }

    void CollisionTree::PairIterator::increment_current()
    {
        if (second_.done())
        {
            current_.first = first_.next();
            if (!first_.done())
            {
                second_.set(first_);
                current_.second = second_.next();
            }
            else
                current_.first = current_.second = nullptr;
        }
        else
            current_.second = second_.next();
    }

    void CollisionTree::PairIterator::assert_valid() const
    {
        if (!tree_)
            throw Error(ErrorCode::InvalidIterator);
    }

    void CollisionTree::PairIterator::invalidate() const
    {
        tree_->pair_iterators_.erase(this);
        tree_ = nullptr;
    }

	CollisionTree::CollisionTree(math::Rect2D bounds, glm::uvec2 degree_, size_t cell_capacity_)
		: degree_(degree_)
        , inv_degree_(1.0f / glm::vec2(degree_))
        , cell_capacity_(cell_capacity_)
	{
		OLY_ASSERT(degree_.x * degree_.y >= 2);
		OLY_ASSERT(cell_capacity_ >= 2);
        root_ = internal::CollisionNode::instantiate(this, bounds);
	}

	CollisionTree::CollisionTree(const CollisionTree& o)
		: cell_capacity_(o.cell_capacity_)
        , degree_(o.degree_)
        , inv_degree_(o.inv_degree_)
	{
        root_ = std::unique_ptr<internal::CollisionNode>(new internal::CollisionNode(this, nullptr, *o.root_));
	}

	CollisionTree::CollisionTree(CollisionTree&& o) noexcept
		: cell_capacity_(o.cell_capacity_)
        , degree_(o.degree_)
        , inv_degree_(o.inv_degree_)
	{
        root_ = std::move(o.root_);
        root_->assign_tree(this);
	}

	CollisionTree& CollisionTree::operator=(const CollisionTree& o)
	{
		if (this != &o)
		{
			cell_capacity_ = o.cell_capacity_;
			degree_ = o.degree_;
			inv_degree_ = o.inv_degree_;
            root_ = std::unique_ptr<internal::CollisionNode>(new internal::CollisionNode(this, nullptr, *o.root_));
		}
		return *this;
	}

	CollisionTree& CollisionTree::operator=(CollisionTree&& o) noexcept
	{
		if (this != &o)
		{
			cell_capacity_ = o.cell_capacity_;
			degree_ = o.degree_;
			inv_degree_ = o.inv_degree_;
            root_ = std::move(o.root_);
            root_->assign_tree(this);
		}
		return *this;
	}

    CollisionTree::BFSColliderIterator CollisionTree::query(const Collider& collider) const
    {
        return BFSColliderIterator(*this, collider.quad_wrap_);
    }

    CollisionTree::BFSColliderIterator CollisionTree::query(const math::Rect2D bounds) const
    {
        return BFSColliderIterator(*this, bounds);
    }

    CollisionTree::PairIterator CollisionTree::iterator() const
    {
        return PairIterator(*this, root_->bounds_);
    }

    void CollisionTree::set_bounds(math::Rect2D bounds)
    {
        root_->set_bounds(bounds);
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
		std::queue<internal::CollisionNode*> nodes_;
		nodes_.push(root_.get());
		while (!nodes_.empty())
		{
			internal::CollisionNode* node = nodes_.front();
			nodes_.pop();

			for (const Collider* collider : node->get_colliders())
				collider->flush();

            for (std::unique_ptr<internal::CollisionNode>& subnode : node->subnodes_)
            {
				if (internal::CollisionNode* sub = subnode.get())
					nodes_.push(sub);
            }
		}
	}

	void CollisionTree::flush_insert_downward() const
	{
		// Sensitive BFS
		std::queue<internal::CollisionNode*> nodes_;
		nodes_.push(root_.get());
		while (!nodes_.empty())
		{
			internal::CollisionNode* node = nodes_.front();
			nodes_.pop();

			node->subdivide();

            for (std::unique_ptr<internal::CollisionNode>& subnode : node->subnodes_)
            {
				if (internal::CollisionNode* sub = subnode.get())
					nodes_.push(sub);
            }
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

			Indexer(internal::CollisionNode* node, size_t index_in_parent, bool visited_children)
                : node(node)
                , index_in_parent(index_in_parent)
                , visited_children(visited_children)
            {
            }
		};

		std::stack<Indexer> stack;
		stack.emplace(root_.get(), 0, false);
		while (!stack.empty())
		{
			Indexer indexer = stack.top();
			stack.pop();

			if (indexer.visited_children)
			{
				if (internal::CollisionNode* parent = indexer.node->parent_) // non-root
				{
					// due to traversal, it is guaranteed that node has no subnodes
					if (parent->get_colliders().size() < cell_capacity_)
					{
						// transfer some colliders over to parent from node
						const size_t transfers = std::min(
                            cell_capacity_ - parent->get_colliders().size(),
                            indexer.node->get_colliders().size());

						for (size_t _ = 0; _ < transfers; ++_)
						{
							if (const Collider* collider = indexer.node->set_colliders().pop())
							{
								parent->set_colliders().insert(collider);
                                if (auto ptr = collider->handles.map_.try_get(this))
                                    *ptr = parent;
                                else
                                    throw Error(ErrorCode::NullPointer);
							}
						}
					}

					if (indexer.node->get_colliders().empty())
					{
						// node is an empty subnode
						parent->subnodes_[indexer.index_in_parent].reset();
					}
				}
			}
			else
			{
				stack.emplace(indexer.node, indexer.index_in_parent, true);
				auto& subnodes = indexer.node->subnodes_;
				for (size_t i = 0; i < subnodes.size(); ++i)
				{
					size_t idx = subnodes.size() - 1 - i;
					if (subnodes[idx].get())
						stack.emplace(subnodes[idx].get(), idx, false);
				}
			}
		}
	}

	void CollisionTree::invalidate_iterators() const
	{
		while (!bfs_collider_iterators_.empty())
			(*bfs_collider_iterators_.begin())->invalidate();
		while (!pair_iterators_.empty())
			(*bfs_collider_iterators_.begin())->invalidate();
	}
}
