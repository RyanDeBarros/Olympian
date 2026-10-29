#pragma once

#include "physics/collision/Tolerance.h"

#include "physics/collision/scene/luts/LUT.h"
#include "physics/collision/scene/luts/LUTVariant.h"

#include <imp/contiguous_set.hpp>
#include <imp/fixed_vector.hpp>

#include <memory>
#include <queue>

namespace oly::col2d
{
    class Collider;
    class CollisionTree;

    namespace internal
    {
        class TreeHandleMap;

        class CollisionNode
        {
            friend class CollisionTree;
            friend class Collider;
            friend class internal::TreeHandleMap;

        private:
            const CollisionTree* tree_;
            math::Rect2D bounds_;
            CollisionNode* parent_ = nullptr;
            imp::fixed_vector<std::unique_ptr<CollisionNode>> subnodes_;
            imp::contiguous_set<const Collider*> colliders_;

            CollisionNode(const CollisionTree* tree, math::Rect2D bounds);
            CollisionNode(const CollisionTree* tree, CollisionNode* parent, const CollisionNode& o);

        public:
            ~CollisionNode();

        public:
            imp::contiguous_set<const Collider*>& set_colliders();
            const imp::contiguous_set<const Collider*>& get_colliders() const;

        private:
            static std::unique_ptr<CollisionNode> instantiate(const CollisionTree* tree, math::Rect2D bounds);

            void assign_tree(const CollisionTree* new_tree);
            
            void update(const Collider& collider, CollisionNode*& node);
            void insert_upwards(const Collider& collider, CollisionNode*& node);

            void subdivide();

            bool subnode_coordinates(math::Rect2D b, unsigned int& x, unsigned int& y) const;
            size_t idx(unsigned int x, unsigned int y) const;

            math::Rect2D subdivision(int x, int y) const;

            void set_bounds(math::Rect2D b);
        };
    }

    class CollisionDispatcher;

    class CollisionTree
    {
        friend class internal::CollisionNode;
        friend class internal::TreeHandleMap;
        friend class Collider;
        friend class CollisionDispatcher;

        class BFSColliderIterator
        {
            friend class CollisionTree;

        private:
            mutable const CollisionTree* tree_ = nullptr;
            math::Rect2D bounds_;
            std::queue<const internal::CollisionNode*> nodes_;
            size_t i_ = 0;
            const Collider* current_ = nullptr;

        public:
            BFSColliderIterator(const math::Rect2D bounds);
            BFSColliderIterator(const CollisionTree& tree, const math::Rect2D bounds);
            BFSColliderIterator(const BFSColliderIterator&);
            BFSColliderIterator(BFSColliderIterator&&);
            ~BFSColliderIterator();

            BFSColliderIterator& operator=(const BFSColliderIterator&);
            BFSColliderIterator& operator=(BFSColliderIterator&&);

        public:
            bool done() const;
            const Collider* next();

        private:
            void set(const BFSColliderIterator&);
            void increment_current();
            void assert_valid() const;
            void invalidate() const;
        };

        class PairIterator
        {
            friend class CollisionTree;

            struct ColliderPtrPair
            {
                const Collider* first = nullptr;
                const Collider* second = nullptr;

                operator bool() const;
            };

        private:
            mutable const CollisionTree* tree_ = nullptr;
            BFSColliderIterator first_;
            BFSColliderIterator second_;
            ColliderPtrPair current_;

        public:
            PairIterator(const CollisionTree& tree, const math::Rect2D bounds);
            PairIterator(const PairIterator&);
            PairIterator(PairIterator&&);
            ~PairIterator();

            PairIterator& operator=(const PairIterator&);
            PairIterator& operator=(PairIterator&&);

        public:
            bool done() const;
            ColliderPtrPair next();

        private:
            void increment_current();
            void assert_valid() const;
            void invalidate() const;
        };

    private:
        size_t cell_capacity_;
        glm::uvec2 degree_;
        glm::vec2 inv_degree_;
        std::unique_ptr<internal::CollisionNode> root_;
        mutable std::unordered_set<const BFSColliderIterator*> bfs_collider_iterators_;
        mutable std::unordered_set<const PairIterator*> pair_iterators_;

    public:
        CollisionTree(math::Rect2D bounds, glm::uvec2 degree = { 2, 2 }, size_t cell_capacity = 4);
        CollisionTree(const CollisionTree&);
        CollisionTree(CollisionTree&&) noexcept;

        CollisionTree& operator=(const CollisionTree&);
        CollisionTree& operator=(CollisionTree&&) noexcept;

    public:
        BFSColliderIterator query(const Collider& collider) const;
        BFSColliderIterator query(const math::Rect2D bounds) const;
        PairIterator iterator() const;

        void set_bounds(math::Rect2D bounds);

    private:
        // call flush() after all collision objects have moved, but before handling events
        void flush() const;

        void flush_update_colliders() const;
        void flush_insert_downward() const;
        void flush_remove_upward() const;

        void invalidate_iterators() const;
    };
}
