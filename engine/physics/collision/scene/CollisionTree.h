#pragma once

#include "core/containers/FixedVector.h"
#include "core/containers/ContiguousSet.h"
#include "core/containers/ContiguousMap.h"
#include "core/types/SoftReference.h"

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
			ContiguousSet<ConstSoftReference<Collider>> colliders;

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

			bool subnode_coordinates(const math::Rect2D& b, unsigned int& x, unsigned int& y) const;
			size_t idx(unsigned int x, unsigned int y) const;

			math::Rect2D subdivision(int x, int y) const;

		public:
			const ContiguousSet<ConstSoftReference<Collider>>& get_colliders() const { return colliders; }
		};
	}

	class CollisionDispatcher;
	class CollisionTree
	{
		friend class internal::CollisionNode;
		friend class internal::TreeHandleMap;
		friend class Collider;
		friend class CollisionDispatcher;

		size_t cell_capacity;
		glm::uvec2 degree;
		glm::vec2 inv_degree;

		mutable std::unique_ptr<internal::CollisionNode> root;

	public:
		CollisionTree(math::Rect2D bounds, glm::uvec2 degree = { 2, 2 }, size_t cell_capacity = 4);
		CollisionTree(const CollisionTree&);
		CollisionTree(CollisionTree&&) noexcept;
		~CollisionTree();
		CollisionTree& operator=(const CollisionTree&);
		CollisionTree& operator=(CollisionTree&&) noexcept;

	private:
		// call flush() after all collision objects have moved, but before handling events
		void flush() const;

		void flush_update_colliders() const;
		void flush_insert_downward() const;
		void flush_remove_upward() const;

		class BFSIterator
		{
			friend class CollisionTree;
			std::queue<internal::CollisionNode*> nodes;
			BFSIterator(internal::CollisionNode* root) { nodes.push(root); }

		public:
			bool done() const { return nodes.empty(); }
			internal::CollisionNode* next();
		};

		class BFSColliderIterator
		{
			friend class CollisionTree;
			const math::Rect2D bounds;
			std::queue<const internal::CollisionNode*> nodes;
			size_t i = 0;
			ConstSoftReference<Collider> current = nullptr;
			BFSColliderIterator(const math::Rect2D bounds) : bounds(bounds) {}
			BFSColliderIterator(const internal::CollisionNode* root, const math::Rect2D bounds);

			void set(const BFSColliderIterator&);

			void increment_current();

		public:
			bool done() const { return !current; }
			ConstSoftReference<Collider> next();
		};

		class PairIterator
		{
			friend class CollisionTree;

			BFSColliderIterator first, second;
			struct ColliderPtrPair
			{
				ConstSoftReference<Collider> first = nullptr;
				ConstSoftReference<Collider> second = nullptr;

				operator bool () const { return first && second; }
			} current;

			PairIterator(internal::CollisionNode* node, const math::Rect2D bounds);

			void increment_current();

		public:
			bool done() const { return !current; }
			ColliderPtrPair next();
		};

	public:
		BFSColliderIterator query(const Collider& collider) const;
		BFSColliderIterator query(const math::Rect2D bounds) const;
		PairIterator iterator() const { return PairIterator(root.get(), root->bounds); }
	};
}
