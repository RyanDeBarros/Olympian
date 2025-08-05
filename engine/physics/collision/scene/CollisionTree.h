#pragma once

#include "core/containers/FixedVector.h"
#include "core/containers/ContiguousSet.h"
#include "core/containers/ContiguousMap.h"

#include "physics/collision/scene/LUT.h"
#include "physics/collision/scene/LUTVariant.h"
#include "physics/collision/Tolerance.h"

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

			const CollisionTree* tree;
			math::Rect2D bounds;
			CollisionNode* parent = nullptr;
			FixedVector<std::unique_ptr<CollisionNode>> subnodes;
			ContiguousSet<const Collider*> _colliders;

			CollisionNode(const CollisionTree* tree, math::Rect2D bounds);
			CollisionNode(const CollisionTree* tree, CollisionNode* parent, const CollisionNode& other);
			void assign_tree(const CollisionTree* new_tree);

		public:
			~CollisionNode();

		private:
			static std::unique_ptr<CollisionNode> instantiate(const CollisionTree* tree, math::Rect2D bounds);

			void update(const Collider& collider, CollisionNode*& node);
			void insert_upwards(const Collider& collider, CollisionNode*& node);

			void subdivide();

			bool subnode_coordinates(math::Rect2D b, unsigned int& x, unsigned int& y) const;
			size_t idx(unsigned int x, unsigned int y) const;

			math::Rect2D subdivision(int x, int y) const;

			void set_bounds(math::Rect2D b);

		public:
			ContiguousSet<const Collider*>& set_colliders();
			const ContiguousSet<const Collider*>& get_colliders() const { return _colliders; }
		};
	}

	namespace internal
	{
		class CollisionDispatcher;
	}

	class CollisionTree
	{
		friend class internal::CollisionNode;
		friend class internal::TreeHandleMap;
		friend class Collider;
		friend class internal::CollisionDispatcher;

		size_t cell_capacity;
		glm::uvec2 degree;
		glm::vec2 inv_degree;

		std::unique_ptr<internal::CollisionNode> root;

	public:
		CollisionTree(math::Rect2D bounds, glm::uvec2 degree = { 2, 2 }, size_t cell_capacity = 4);
		CollisionTree(const CollisionTree&);
		CollisionTree(CollisionTree&&) noexcept;
		CollisionTree& operator=(const CollisionTree&);
		CollisionTree& operator=(CollisionTree&&) noexcept;

	private:
		// call flush() after all collision objects have moved, but before handling events
		void flush() const;

		void flush_update_colliders() const;
		void flush_insert_downward() const;
		void flush_remove_upward() const;

		void invalidate_iterators() const;

		class BFSColliderIterator
		{
			friend class CollisionTree;
			mutable const CollisionTree* tree = nullptr;

			math::Rect2D bounds;
			std::queue<const internal::CollisionNode*> nodes;
			size_t i = 0;
			const Collider* current = nullptr;
			
		public:
			BFSColliderIterator(const math::Rect2D bounds) : bounds(bounds) {}
			BFSColliderIterator(const CollisionTree& tree, const math::Rect2D bounds);
			BFSColliderIterator(const BFSColliderIterator&);
			BFSColliderIterator(BFSColliderIterator&&);
			~BFSColliderIterator();
			BFSColliderIterator& operator=(const BFSColliderIterator&);
			BFSColliderIterator& operator=(BFSColliderIterator&&);

		private:
			void set(const BFSColliderIterator&);

			void increment_current();

		public:
			bool done() const { return !tree || !current; }
			const Collider* next();

		private:
			void assert_valid() const;
			void invalidate() const;
		};

		mutable std::unordered_set<const BFSColliderIterator*> bfs_collider_iterators;

		class PairIterator
		{
			friend class CollisionTree;
			mutable const CollisionTree* tree = nullptr;

			BFSColliderIterator first, second;
			struct ColliderPtrPair
			{
				const Collider* first = nullptr;
				const Collider* second = nullptr;

				operator bool () const { return first && second; }
			} current;

		public:
			PairIterator(const CollisionTree& tree, const math::Rect2D bounds);
			PairIterator(const PairIterator&);
			PairIterator(PairIterator&&);
			~PairIterator();
			PairIterator& operator=(const PairIterator&);
			PairIterator& operator=(PairIterator&&);

		private:
			void increment_current();

		public:
			bool done() const { return !tree || !current; }
			ColliderPtrPair next();

		private:
			void assert_valid() const;
			void invalidate() const;
		};

		mutable std::unordered_set<const PairIterator*> pair_iterators;

	public:
		BFSColliderIterator query(const Collider& collider) const;
		BFSColliderIterator query(const math::Rect2D bounds) const;
		PairIterator iterator() const;
		
		void set_bounds(math::Rect2D bounds);
	};
}
