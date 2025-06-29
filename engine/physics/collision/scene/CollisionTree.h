#pragma once

#include "core/containers/FixedVector.h"
#include "core/containers/ContiguousSet.h"
#include "core/math/Shapes.h"

#include "physics/collision/Tolerance.h"

#include <memory>

namespace oly::col2d
{
	class CollisionNode;
	class CollisionTree;

	class Collider
	{
		friend class CollisionTree;
		friend class CollisionNode;

		CollisionTree* tree = nullptr;
		mutable CollisionNode* node = nullptr;
		mutable bool dirty = true;

	protected:
		mutable math::Rect2D quad_wrap;

	public:
		Collider(CollisionTree& tree);
		Collider(const Collider&);
		Collider(Collider&&) noexcept;
		virtual ~Collider();
		Collider& operator=(const Collider&);
		Collider& operator=(Collider&&) noexcept;

	private:
		void replace_in_node(Collider&& other) noexcept;
		bool is_dirty() const { return dirty || dirty_impl(); }
		void flush() const;

	protected:
		void flag() const { dirty = true; }
		virtual bool dirty_impl() const { return false; }
		virtual void flush_impl() const {}
	};

	class CollisionNode
	{
		friend class CollisionTree;
		friend class Collider;

		CollisionTree& tree;
		math::Rect2D bounds;
		CollisionNode* parent = nullptr;
		FixedVector<std::unique_ptr<CollisionNode>> subnodes;
		ContiguousSet<const Collider*> colliders;

		CollisionNode(CollisionTree& tree);
		static std::unique_ptr<CollisionNode> instantiate(CollisionTree& tree);

		void insert(const Collider& collider);
		void remove(const Collider& collider);

		void update(const Collider& collider);
		void insert_upwards(const Collider& collider);

		void subdivide();

		bool subnode_coordinates(const math::Rect2D& b, unsigned int& x, unsigned int& y) const;
		const std::unique_ptr<CollisionNode>& subnode(unsigned int x, unsigned int y) const;
		std::unique_ptr<CollisionNode>& subnode(unsigned int x, unsigned int y);

		math::Rect2D subdivision(int x, int y) const;
	};

	class CollisionTree
	{
		friend class CollisionNode;
		friend class Collider;

		const size_t cell_capacity = 4;
		const glm::uvec2 degree;
		const glm::vec2 inv_degree;

		mutable std::unique_ptr<CollisionNode> root;

		void insert(const Collider& collider);

	public:
		CollisionTree(math::Rect2D bounds, glm::uvec2 degree = { 2, 2 }, size_t cell_capacity = 4);

		void flush() const;

	private:
		void flush_update_colliders() const;
		void flush_insert_downward() const;
		void flush_remove_upward() const;
	};
}
