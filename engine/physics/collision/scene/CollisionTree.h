#pragma once

#include "core/containers/FixedVector.h"
#include "core/containers/ContiguousSet.h"
#include "core/types/SoftReference.h"

#include "physics/collision/scene/LUT.h"
#include "physics/collision/Tolerance.h"

#include <memory>
#include <queue>

namespace oly::col2d
{
	class CollisionNode;
	class CollisionTree;

	class Collider
	{
		// LATER movable/static Colliders for optimization in CollisionTree flushing.
		friend class CollisionTree;
		friend class CollisionNode;

		OLY_SOFT_REFERENCE_BASE_DECLARATION(Collider);

	private:
		internal::ColliderObject obj;

		mutable CollisionTree* tree = nullptr;
		mutable CollisionNode* node = nullptr;
		mutable bool dirty = true;

	protected:
		mutable math::Rect2D quad_wrap;

	public:
		template<typename CObj>
		explicit Collider(CObj&& obj, CollisionTree* tree = nullptr) : obj(std::move(obj)), tree(tree) {}
		Collider(internal::ColliderObject&& obj, CollisionTree* tree = nullptr) : obj(std::move(obj)), tree(tree) {}
		Collider(const Collider&);
		Collider(Collider&&) noexcept;
		~Collider();
		Collider& operator=(const Collider&);
		Collider& operator=(Collider&&) noexcept;

		// TODO allow assignment for multiple trees
		const CollisionTree* get_tree() const { return tree; }
		CollisionTree* get_tree() { return tree; }
		void set_tree(CollisionTree* tree);
		void unset_tree();

		const internal::ColliderObject& _obj() const { return obj; }
		template<typename CObj>
		const CObj& get() const { return obj.get<CObj>(); }
		template<typename CObj>
		CObj& set() { dirty = true; return obj.set<CObj>(); }

	private:
		void replace_in_node(Collider&& other) noexcept;
		bool is_dirty() const { return dirty || internal::lut_is_dirty(obj); }
		void flush() const;
	};

#define OLY_COLLIDER_HEADER(Class)\
	OLY_SOFT_REFERENCE_PUBLIC(Class);

	class CollisionNode
	{
		friend class CollisionTree;
		friend class Collider;

		CollisionTree& tree;
		math::Rect2D bounds;
		CollisionNode* parent = nullptr;
		FixedVector<std::unique_ptr<CollisionNode>> subnodes;
		ContiguousSet<ConstSoftReference<Collider>> colliders;

		CollisionNode(CollisionTree& tree);

	public:
		~CollisionNode();
	
	private:
		static std::unique_ptr<CollisionNode> instantiate(CollisionTree& tree);

		void insert(const Collider& collider);
		void remove(const Collider& collider);

		void update(const Collider& collider);
		void insert_upwards(const Collider& collider);

		void subdivide();

		bool subnode_coordinates(const math::Rect2D& b, unsigned int& x, unsigned int& y) const;
		size_t idx(unsigned int x, unsigned int y) const;

		math::Rect2D subdivision(int x, int y) const;

	public:
		const ContiguousSet<ConstSoftReference<Collider>>& get_colliders() const { return colliders; }
	};

	class CollisionTree
	{
		friend class CollisionNode;
		friend class Collider;

		const size_t cell_capacity = 4;
		const glm::uvec2 degree;
		const glm::vec2 inv_degree;

		mutable std::unique_ptr<CollisionNode> root;

	public:
		CollisionTree(math::Rect2D bounds, glm::uvec2 degree = { 2, 2 }, size_t cell_capacity = 4);
		~CollisionTree();

		// call flush() after all collision objects have moved, but before handling events
		void flush() const;

	private:
		void flush_update_colliders() const;
		void flush_insert_downward() const;
		void flush_remove_upward() const;

		class BFSIterator
		{
			friend class CollisionTree;
			std::queue<CollisionNode*> nodes;
			BFSIterator(CollisionNode* root) { nodes.push(root); }

		public:
			bool done() const { return nodes.empty(); }
			CollisionNode* next();
		};

		class BFSColliderIterator
		{
			friend class CollisionTree;
			const math::Rect2D bounds;
			std::queue<const CollisionNode*> nodes;
			size_t i = 0;
			BFSColliderIterator(const math::Rect2D bounds) : bounds(bounds) {}
			BFSColliderIterator(const CollisionNode* root, const math::Rect2D bounds) : bounds(bounds) { nodes.push(root); }

			void set(const BFSColliderIterator&);

		public:
			bool done() const { return nodes.empty(); }
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

			PairIterator(CollisionNode* node, const math::Rect2D bounds);

			void increment_current();

		public:
			bool done() const { return !current; }
			ColliderPtrPair next();
		};

	public:
		BFSColliderIterator query(const Collider& collider) const { return BFSColliderIterator(root.get(), collider.quad_wrap); }
		BFSColliderIterator query(const math::Rect2D bounds) const { return BFSColliderIterator(root.get(), bounds); }
		PairIterator iterator() const { return PairIterator(root.get(), root->bounds); }
	};
}
